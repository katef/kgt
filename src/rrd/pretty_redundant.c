/* $Id$ */

/*
 * Railroad diagram beautification
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static int
node_walk(struct node **n);

static int
redundant_alt(struct node *n, struct node **np)
{
	int nc = 0, isopt = 0;
	struct list **p;
	struct node **loop;

	loop = NULL;

	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		nc++;

		if (!node_walk(&(*p)->node)) {
			return 0;
		}

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
		} else if (l->u.loop.backward->type == NODE_SKIP) {
			struct node *tmp;

			tmp = l->u.loop.backward;
			l->u.loop.backward = l->u.loop.forward;
			l->u.loop.forward = tmp;
			*np = *loop;
			*loop = NULL;

			node_free(n);
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
		}
	}

	return 1;
}

static int
redundant_loop(struct node *n, struct node **np)
{
	struct node **inner = NULL;
	struct node *loop;

	if (!node_walk(&n->u.loop.forward)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward)) {
		return 0;
	}

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
	}

	return 1;
}

static int
node_walk(struct node **n)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
		struct list **p;

	case NODE_ALT:
		return redundant_alt(node, n);

	case NODE_LOOP:
		return redundant_loop(node, n);

	case NODE_SEQ:
		for (p = &node->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(&(*p)->node)) {
				return 0;
			}
		}

		break;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		break;
	}

	return 1;
}

void
rrd_pretty_redundant(struct node **rrd)
{
	node_walk(rrd);
}

