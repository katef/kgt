/* $Id$ */

/*
 * Railroad diagram beautification
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static int
node_walk(struct node **n);

static int
bottom_seq(struct node *n)
{
	struct list **p;

	assert(n != NULL);

	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		if (!node_walk(&(*p)->node)) {
			return 0;
		}
	}

	return 1;
}

static int
bottom_loop(struct node **np)
{
	struct node *n = *np;
	int everything;

	assert(n != NULL);

	do {
		struct list *new;

		if (n->u.loop.forward->type != NODE_SKIP) {
			break;
		}

		if (n->u.loop.backward->type == NODE_SKIP
		 || n->u.loop.backward->type == NODE_LOOP) {
			break;
		}

		if (n->u.loop.backward->type == NODE_LITERAL || n->u.loop.backward->type == NODE_RULE) {
			break;
		}

		if (n->u.loop.backward->type == NODE_ALT) {
			struct list *p;
			int c;

			c = 0;

			for (p = n->u.loop.backward->u.alt; p != NULL; p = p->next) {
				if (p->node->type == NODE_ALT || p->node->type == NODE_SEQ || p->node->type == NODE_LOOP) {
					c = 1;
				}
			}

			if (!c) {
				break;
			}
		}

		{
			struct node *tmp;

			tmp = n->u.loop.backward;
			n->u.loop.backward = n->u.loop.forward;
			n->u.loop.forward  = tmp;
		}

		/* short-circuit */
		{
			struct list *new;

			new  = NULL;

			list_push(&new, n);
			list_push(&new, node_create_skip());

			*np = node_create_alt(new);
		}

		return 1;
	} while (0);

	if (!node_walk(&n->u.loop.forward)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward)) {
		return 0;
	}

	return 1;
}

static int
node_walk(struct node **n)
{
	assert(n != NULL);

	switch ((*n)->type) {
		struct list **p;

	case NODE_SEQ:
		return bottom_seq(*n);

	case NODE_LOOP:
		return bottom_loop(n);

	case NODE_ALT:
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
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

/*
 * for loops with nothing on top and more than one thing on the bottom,
 * flip the loop over and add an alt to skip the loop altogether.
 * this results in a bulkier diagram, but avoids reversing the contents of
 * the sequence.
 */
void
rrd_pretty_bottom(struct node **rrd)
{
	node_walk(rrd);
}

