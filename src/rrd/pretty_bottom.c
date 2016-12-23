/* $Id$ */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static int
bottom_loop(struct node **np)
{
    struct node *n;

	assert(np != NULL);
	assert(*np != NULL);

	n = *np;

	if (n->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (n->u.loop.backward->type == NODE_SKIP
	 || n->u.loop.backward->type == NODE_LOOP) {
		return 0;
	}

	if (n->u.loop.backward->type == NODE_LITERAL || n->u.loop.backward->type == NODE_RULE) {
		return 0;
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
			return 0;
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
}

static void
node_walk(struct node **n)
{
	assert(n != NULL);

	switch ((*n)->type) {
		struct list **p;

	case NODE_ALT:
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(&(*p)->node);
		}

		break;

	case NODE_SEQ:
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(&(*p)->node);
		}

		break;

	case NODE_LOOP:
		if (bottom_loop(n)) {
			/* node changed */
			node_walk(n);
			return;
		}

		node_walk(&(*n)->u.loop.forward);
		node_walk(&(*n)->u.loop.backward);

		break;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		break;
	}
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

