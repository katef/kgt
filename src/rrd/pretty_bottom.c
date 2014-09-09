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

static struct node_walker pretty_bottom;

struct bottom_context {
	int applied;
	int everything;
};

static int
bottom_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	struct bottom_context *ctx = arg;
	struct node **p;
	int anything = 0;

	(void) np;

	for (p = &n->u.sequence; *p != NULL; p = &(**p).next) {
		ctx->applied = 0;

		if (!node_walk(p, &pretty_bottom, depth + 1, ctx)) {
			return 0;
		}

		anything = anything || ctx->applied;
	}

	if (0 && anything) {
		ctx->everything = 1;
		node_walk_list(&n->u.sequence, &pretty_bottom, depth + 1, ctx);
		ctx->everything = 0;
	}

	return 1;
}

static int
bottom_loop(struct node *n, struct node **np, int depth, void *arg)
{
	struct bottom_context *ctx = arg;
	int everything = ctx->everything;
	ctx->everything = 0;

	do {
		struct node *choice;
		struct node *tmp, *skip;

		if (n->u.loop.forward->type != NODE_SKIP) {
			break;
		}

		if (n->u.loop.backward->type == NODE_SKIP
		 || n->u.loop.backward->type == NODE_LOOP) {
			break;
		}

		if (!everything && (n->u.loop.backward->type == NODE_LITERAL || n->u.loop.backward->type == NODE_RULE)) {
			break;
		}

		if (n->u.loop.backward->type == NODE_CHOICE) {
			struct node *p;
			int c;

			c = 0;

			for (p = n->u.loop.backward->u.choice; p != NULL; p = p->next) {
				if (p->type == NODE_CHOICE || p->type == NODE_SEQUENCE || p->type == NODE_LOOP) {
					c = 1;
				}
			}

			if (!c) {
				break;
			}
		}

		tmp = n->u.loop.backward;
		n->u.loop.backward = n->u.loop.forward;
		n->u.loop.forward  = tmp;

		/* short-circuit */
		skip = node_create_skip();
		skip->next = n;
		choice = node_create_choice(skip);
		choice->next = n->next;
		n->next = NULL;
		*np = choice;

		if (!node_walk(&skip->next, &pretty_bottom, depth + 1, ctx)) {
			return 0;
		}

		ctx->applied = 1;
		ctx->everything = everything;
		return 1;
	} while (0);

	if (!node_walk(&n->u.loop.forward, &pretty_bottom, depth + 1, ctx)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, &pretty_bottom, depth + 1, ctx)) {
		return 0;
	}

	ctx->applied = 0;
	ctx->everything = everything;

	return 1;
}

static struct node_walker pretty_bottom = {
	NULL,
	NULL, NULL,
	NULL, bottom_sequence,
	bottom_loop
};

void
rrd_pretty_bottom(struct node **rrd)
{
	/*
	 * for loops with nothing on top and more than one thing on the bottom,
	 * flip the loop over and add a choice to skip the loop altogether.
	 * this results in a bulkier diagram, but avoids reversing the contents of
	 * the sequence.
	 */
	struct bottom_context ctx = { 0, 0 };
	node_walk(rrd, &pretty_bottom, 0, &ctx);
}

