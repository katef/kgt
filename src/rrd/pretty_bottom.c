/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <string.h>
#include <stddef.h>
#include <assert.h>

#include "../txt.h"

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

	if (n->u.loop.forward != NULL) {
		return 0;
	}

	if (n->u.loop.backward == NULL || n->u.loop.backward->type == NODE_LOOP) {
		return 0;
	}

	if (n->u.loop.backward->type == NODE_CI_LITERAL || n->u.loop.backward->type == NODE_CS_LITERAL || n->u.loop.backward->type == NODE_RULE) {
		return 0;
	}

	if (n->u.loop.backward->type == NODE_ALT || n->u.loop.backward->type == NODE_ALT_SKIPPABLE) {
		struct list *p;
		int c;

		c = 0;

		for (p = n->u.loop.backward->u.alt; p != NULL; p = p->next) {
			if (p->node->type == NODE_ALT || p->node->type == NODE_ALT_SKIPPABLE || p->node->type == NODE_SEQ || p->node->type == NODE_LOOP) {
				c = 1;
			}
		}

		if (!c) {
			return 0;
		}
	}

	loop_flip(n);

	/* short-circuit */
	{
		struct list *new;

		new  = NULL;

		list_push(&new, n);
		list_push(&new, NULL);

		*np = node_create_alt(n->invisible, new);
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
rrd_pretty_bottom(int *changed, struct node **n)
{
	assert(n != NULL);

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
	case NODE_LOOP:
		if (bottom_loop(n)) {
			*changed = 1;
		}
		break;

	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
	case NODE_SEQ:
		break;
	}
}

