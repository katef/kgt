/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>

#include "../txt.h"
#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
redundant_alt(int *changed, struct node *n, struct node **np, int isskippable)
{
	int nc = 0;
	struct list **p;
	struct node **loop;

	loop = NULL;

	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		nc++;

		if ((*p)->node != NULL && (*p)->node->type == NODE_LOOP) {
			loop = &(*p)->node;
		}
	}

	if (nc == 2 && isskippable && loop != NULL) {
		struct node *l;

		l = *loop;

		/* special case: if an optional loop has an empty half, we can elide the NODE_ALT */
		if (l->u.loop.forward == NULL) {
			*np = *loop;
			*loop = NULL;
			node_free(n);

			*changed = 1;
		} else if (l->u.loop.backward == NULL) {
			loop_flip(l);

			*np = *loop;
			*loop = NULL;

/* XXX:
			node_free(n);
*/

			*changed = 1;
		}
	}
}

static void
redundant_loop(int *changed, struct node *n, struct node **np)
{
	struct node **inner = NULL;
	struct node *loop;

	if (n->u.loop.forward != NULL && n->u.loop.forward->type == NODE_LOOP && n->u.loop.backward == NULL) {
		loop = n->u.loop.forward;
		if (loop->u.loop.forward == NULL || loop->u.loop.backward == NULL) {
			inner = &n->u.loop.forward;
		}
	} else if (n->u.loop.backward != NULL && n->u.loop.backward->type == NODE_LOOP && n->u.loop.forward == NULL) {
		loop = n->u.loop.backward;
		if (loop->u.loop.forward == NULL) {
			inner = &n->u.loop.backward;
		} else if (loop->u.loop.backward == NULL) {
			loop_flip(loop);
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

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		redundant_alt(changed, *n, n, (*n)->type == NODE_ALT_SKIPPABLE);
		break;

	case NODE_LOOP:
		redundant_loop(changed, *n, n);
		break;

	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_SEQ:
		break;
	}
}

