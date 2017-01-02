/* $Id$ */

#include <assert.h>
#include <stddef.h>

#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
redundant_alt(int *changed, struct node *n, struct node **np)
{
	int nc = 0, isopt = 0;
	struct list **p;
	struct node **loop;

	loop = NULL;

	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		nc++;

		if ((*p)->node->type == NODE_SKIP) {
			isopt = 1;
		}

		if ((*p)->node->type == NODE_LOOP) {
			loop = &(*p)->node;
		}
	}

	if (nc == 2 && isopt && loop != NULL) {
		struct node *l;

		l = *loop;

		/* special case: if an optional loop has an empty half, we can elide the NODE_ALT */
		if (l->u.loop.forward->type == NODE_SKIP) {
			*np = *loop;
			*loop = NULL;
			node_free(n);

			*changed = 1;
		} else if (l->u.loop.backward->type == NODE_SKIP) {
			struct node *tmp;

			tmp = l->u.loop.backward;
			l->u.loop.backward = l->u.loop.forward;
			l->u.loop.forward = tmp;
			*np = *loop;
			*loop = NULL;

/* XXX:
			node_free(n);
*/

			*changed = 1;
		}
	} else {
		struct list **next;

		/* fold nested alts into this one */
		for (p = &n->u.alt; *p != NULL; p = next) {
			struct list **head, **tail;
			struct list *dead;

			next = &(*p)->next;

			if ((*p)->node->type != NODE_ALT) {
				continue;
			}

			dead = *p;

			/* incoming inner list */
			head = &(*p)->node->u.alt;

			for (tail = head; *tail != NULL; tail = &(*tail)->next)
				;

			*tail = (*p)->next;
			(*p)->next = NULL;

			*p = *head;
			*head = NULL;

			next = p;

			node_free(dead->node);
			list_free(&dead);

			*changed = 1;
		}
	}
}

static void
redundant_loop(int *changed, struct node *n, struct node **np)
{
	struct node **inner = NULL;
	struct node *loop;

	if (n->u.loop.forward->type == NODE_LOOP && n->u.loop.backward->type == NODE_SKIP) {
		loop = n->u.loop.forward;
		if (loop->u.loop.forward->type == NODE_SKIP || loop->u.loop.backward->type == NODE_SKIP) {
			inner = &n->u.loop.forward;
		}
	} else if (n->u.loop.backward->type == NODE_LOOP && n->u.loop.forward->type == NODE_SKIP) {
		loop = n->u.loop.backward;
		if (loop->u.loop.forward->type == NODE_SKIP) {
			inner = &n->u.loop.backward;
		} else if (loop->u.loop.backward->type == NODE_SKIP) {
			struct node *tmp;
			tmp = loop->u.loop.backward;
			loop->u.loop.backward = loop->u.loop.forward;
			loop->u.loop.forward = tmp;
			inner = &n->u.loop.backward;
		}
	}

	if (inner) {
		*np = *inner;
		*inner = NULL;
		node_free(n);
		*changed = 1;
	}
}

void
rrd_pretty_redundant(int *changed, struct node **n)
{
	assert(n != NULL);
	assert(*n != NULL);

	switch ((*n)->type) {
	case NODE_ALT:
		redundant_alt(changed, *n, n);
		break;

	case NODE_LOOP:
		redundant_loop(changed, *n, n);
		break;
	}
}

