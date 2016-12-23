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
node_walk(struct node **n, void *opaque);

struct bottom_context {
	int applied;
	int everything;
};

static int
bottom_seq(struct node *n, void *opaque)
{
	struct bottom_context *ctx = opaque;
	struct list **p;
	int anything = 0;

	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		ctx->applied = 0;

		if (!node_walk(&(*p)->node, ctx)) {
			return 0;
		}

		anything = anything || ctx->applied;
	}

	if (0 && anything) {
		ctx->everything = 1;

		for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(&(*p)->node, ctx)) {
				/* XXX: handle? */
			}
        }

		ctx->everything = 0;
	}

	return 1;
}

static int
bottom_loop(struct node **np, void *opaque)
{
	struct node *n = *np;
	struct bottom_context *ctx = opaque;
	int everything = ctx->everything;
	ctx->everything = 0;

	do {
		struct node *alt;
		struct list *new;

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
			struct list *a;

			a = xmalloc(sizeof *a);
			a->node = n;
			a->next = NULL;

			new = xmalloc(sizeof *new);
			new->node = node_create_skip();
			new->next = a;

			alt = node_create_alt(new);
			*np = alt;
		}

		if (!node_walk(&new->next->node, ctx)) {
			return 0;
		}

		ctx->applied = 1;
		ctx->everything = everything;
		return 1;
	} while (0);

	if (!node_walk(&n->u.loop.forward, ctx)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, ctx)) {
		return 0;
	}

	ctx->applied = 0;
	ctx->everything = everything;

	return 1;
}

static int
node_walk(struct node **n, void *opaque)
{
	assert(n != NULL);

	switch ((*n)->type) {
		struct list **p;

	case NODE_SEQ:
		return bottom_seq(*n, opaque);

	case NODE_LOOP:
		return bottom_loop(n, opaque);

	case NODE_ALT:
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			if (!node_walk(&(*p)->node, opaque)) {
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
	struct bottom_context ctx = { 0, 0 };
	node_walk(rrd, &ctx);
}

