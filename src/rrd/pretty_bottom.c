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

static int
node_walk(struct node **n, int depth, void *opaque);

struct bottom_context {
	int applied;
	int everything;
};

static int
bottom_seq(struct node *n, struct node **np, int depth, void *opaque)
{
	struct bottom_context *ctx = opaque;
	struct node **p;
	int anything = 0;

	(void) np;

	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		ctx->applied = 0;

		if (!node_walk(p, depth + 1, ctx)) {
			return 0;
		}

		anything = anything || ctx->applied;
	}

	if (0 && anything) {
		ctx->everything = 1;

		for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, depth + 1, ctx)) {
				/* XXX: handle? */
			}
        }

		ctx->everything = 0;
	}

	return 1;
}

static int
bottom_loop(struct node *n, struct node **np, int depth, void *opaque)
{
	struct bottom_context *ctx = opaque;
	int everything = ctx->everything;
	ctx->everything = 0;

	do {
		struct node *alt;
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

		if (n->u.loop.backward->type == NODE_ALT) {
			struct node *p;
			int c;

			c = 0;

			for (p = n->u.loop.backward->u.alt; p != NULL; p = p->next) {
				if (p->type == NODE_ALT || p->type == NODE_SEQ || p->type == NODE_LOOP) {
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
		alt = node_create_alt(skip);
		alt->next = n->next;
		n->next = NULL;
		*np = alt;

		if (!node_walk(&skip->next, depth + 1, ctx)) {
			return 0;
		}

		ctx->applied = 1;
		ctx->everything = everything;
		return 1;
	} while (0);

	if (!node_walk(&n->u.loop.forward, depth + 1, ctx)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, depth + 1, ctx)) {
		return 0;
	}

	ctx->applied = 0;
	ctx->everything = everything;

	return 1;
}

static int
node_walk(struct node **n, int depth, void *opaque)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SEQ:  return bottom_seq(node, n, depth, opaque);
	case NODE_LOOP: return bottom_loop(node, n, depth, opaque);
	}

	switch (node->type) {
		struct node **p;

	case NODE_ALT:
		for (p = &node->u.alt; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_SEQ:
		for (p = &node->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_LOOP:
		if (!node_walk(&node->u.loop.forward, depth + 1, opaque)) {
			return 0;
		}

		if (!node_walk(&node->u.loop.backward, depth + 1, opaque)) {
			return 0;
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
	struct bottom_context ctx = { 0, 0 };
	node_walk(rrd, 0, &ctx);
}

