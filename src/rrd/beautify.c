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
#include "blist.h"
#include "node.h"

static int
leaves_eq(struct node *a, struct node *b)
{
	assert(a->type == NODE_LEAF);
	assert(b->type == NODE_LEAF);

	if (a->u.leaf.type != b->u.leaf.type) {
		return 0;
	}

	if (0 != strcmp(a->u.leaf.text, b->u.leaf.text)) {
		return 0;
	}

	return 1;
}

static int
process_loop_leaf(struct node *loop, struct bnode *bp)
{
	struct node *a, *b;

	if (bp == NULL || bp->v->type != NODE_LEAF) {
		return 0;
	}

	a = loop->u.loop.backward;
	b = bp->v;

	if (leaves_eq(a, b)) {
		struct node *tmp;

		tmp = loop->u.loop.forward;
		loop->u.loop.forward  = loop->u.loop.backward;
		loop->u.loop.backward = tmp;

		return 1;
	}

	return 0;
}

static void
loop_switch_sides(int suflen, struct node *loop, struct bnode **rl)
{
	struct node *v, **n;
	int i;

	if (suflen > 1) {
		struct node *seq;

		seq = node_create_list(LIST_SEQUENCE, NULL);

		n = &seq->u.list.list;
		node_free(loop->u.loop.forward);
		loop->u.loop.forward = seq;

		for (i = 0; i < suflen; i++) {
			(void) b_pop(rl, &v);
			v->next = *n;
			*n = v;
		}
	} else {
		node_free(loop->u.loop.forward);
		(void) b_pop(rl, &v);
		v->next = 0;
		loop->u.loop.forward = v;
	}

	if (b_pop(rl, &v)) {
		v->next = 0;
	} else {
		struct node *skip;

		skip = node_create_skip();
		node_free(loop->u.loop.backward);
		loop->u.loop.backward = skip;
	}

	if (loop->u.loop.backward->type == NODE_LIST) {
		node_collapse(&loop->u.loop.backward);
	}
}

static int
process_loop_list(struct node *loop, struct bnode *bp)
{
	struct node *list;
	struct node *p;
	struct bnode *rl = 0, *rp;
	int suffix = 0;

	list = loop->u.loop.backward;

	if (list->u.list.type == LIST_CHOICE) {
		return 0;
	}

	for (p = list->u.list.list; p != NULL; p = p->next) {
		b_push(&rl, p);
	}

	/* linkedlistcmp() */
	for (rp = rl; rp != NULL && bp != NULL; rp = rp->next, bp = bp->next) {
		struct node *a, *b;

		if (rp->v->type != NODE_LEAF || bp->v->type != NODE_LEAF) {
			break;
		}

		a = rp->v;
		b = bp->v;

		if (!leaves_eq(a, b)) {
			break;
		}

		suffix++;
	}

	if (suffix > 0) {
		loop_switch_sides(suffix, loop, &rl);
	}

	b_clear(&rl);

	return suffix;
}

static int
process_loop(struct node *loop, struct bnode *bp)
{
	if (loop->u.loop.backward->type == NODE_SKIP
	 || loop->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (loop->u.loop.backward->type == NODE_LIST) {
		return process_loop_list(loop, bp);
	}

	if (loop->u.loop.backward->type == NODE_LEAF) {
		return process_loop_leaf(loop, bp);
	}

	return 0;
}

static struct node_walker bt_collapse_suffixes;

static int
collapse_list(struct node *n, struct node **np, int depth, void *arg)
{
	struct node *p;
	struct bnode *rl = 0;

	for (p = n->u.list.list; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->type != NODE_LOOP) {
			b_push(&rl, p);
			continue;
		}

		suffix_len = process_loop(p, rl);

		/* delete suffix_len things from the list */
		for (i = 0; i < suffix_len; i++) {
			struct node *q;

			if (!b_pop(&rl, &q)) {
				return 0;
			}

			node_free(q);

			if (rl) {
				rl->v->next = p;
			} else {
				n->u.list.list = p;
			}
		}
	}

	b_clear(&rl);

	if (!node_walk_list(&n->u.list.list, &bt_collapse_suffixes, depth + 1, arg)) {
		return 0;
	}

	node_collapse(np);

	return 1;
}

static struct node_walker bt_collapse_suffixes = {
	NULL,
	NULL, NULL,
	NULL, collapse_list,
	NULL
};

static struct node_walker bt_bot_heavy;

struct bot_heavy_context {
	int applied;
	int everything;
};

static int
bot_heavy_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	struct bot_heavy_context *ctx = arg;
	struct node **p;
	int anything = 0;

	(void) np;

	for (p = &n->u.list.list; *p != NULL; p = &(**p).next) {
		ctx->applied = 0;

		if (!node_walk(p, &bt_bot_heavy, depth + 1, ctx)) {
			return 0;
		}

		anything = anything || ctx->applied;
	}

	if (0 && anything) {
		ctx->everything = 1;
		node_walk_list(&n->u.list.list, &bt_bot_heavy, depth + 1, ctx);
		ctx->everything = 0;
	}

	return 1;
}

static int
bot_heavy_loop(struct node *n, struct node **np, int depth, void *arg)
{
	struct bot_heavy_context *ctx = arg;
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

		if (!everything && n->u.loop.backward->type == NODE_LEAF) {
			break;
		}

		if (n->u.loop.backward->type == NODE_LIST) {
			struct node *list;

			list = n->u.loop.backward;
			if (list->u.list.type == LIST_CHOICE) {
				struct node *p;
				int c = 0;

				for (p = list->u.list.list; p != NULL; p = p->next) {
					if (p->type == NODE_LIST || p->type == NODE_LOOP)
						c = 1;
				}

				if (!c) {
					break;
				}
			}
		}

		tmp = n->u.loop.backward;
		n->u.loop.backward = n->u.loop.forward;
		n->u.loop.forward = tmp;

		/* short-circuit */
		skip = node_create_skip();
		skip->next = n;
		choice = node_create_list(LIST_CHOICE, skip);
		choice->next = n->next;
		n->next = NULL;
		*np = choice;

		if (!node_walk(&skip->next, &bt_bot_heavy, depth + 1, ctx)) {
			return 0;
		}

		ctx->applied = 1;
		ctx->everything = everything;
		return 1;
	} while (0);

	if (!node_walk(&n->u.loop.forward, &bt_bot_heavy, depth + 1, ctx)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, &bt_bot_heavy, depth + 1, ctx)) {
		return 0;
	}

	ctx->applied = 0;
	ctx->everything = everything;

	return 1;
}

static struct node_walker bt_bot_heavy = {
	0,
	0, 0,
	0, bot_heavy_sequence,
	bot_heavy_loop
};

static struct node_walker bt_redundant;

static int
redundant_choice(struct node *n, struct node **np, int depth, void *arg)
{
	int nc = 0, isopt = 0;
	struct node **p, **loop = NULL;

	for (p = &n->u.list.list; *p != NULL; p = &(**p).next) {
		nc++;

		if (!node_walk(p, &bt_redundant, depth + 1, arg)) {
			return 0;
		}

		if ((**p).type == NODE_SKIP) {
			isopt = 1;
		}

		if ((**p).type == NODE_LOOP) {
			loop = p;
		}
	}

	if (nc == 2 && isopt && loop != NULL) {
		struct node *l;

		l = *loop;

		/* special case: if an optional loop has an empty half, we can elide the NODE_CHOICE */
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
		struct node **next;

		/* fold nested choices into this one */
		for (p = &n->u.list.list; *p != NULL; p = next) {
			struct node **head, **tail;
			struct node *dead;

			next = &(*p)->next;

			if ((*p)->type != NODE_LIST) {
				continue;
			}

			if ((*p)->u.list.type != LIST_CHOICE) {
				continue;
			}

			dead = *p;

			/* incoming inner list */
			head = &(*p)->u.list.list;

			for (tail = head; *tail != NULL; tail = &(*tail)->next)
				;

			*tail = (*p)->next;
			(*p)->next = NULL;

			*p = *head;
			*head = NULL;

			next = p;

			node_free(dead);
		}
	}

	return 1;
}

static int
redundant_loop(struct node *n, struct node **np, int depth, void *arg)
{
	struct node **inner = 0;
	struct node *loop;

	if (!node_walk(&n->u.loop.forward, &bt_redundant, depth + 1, arg)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, &bt_redundant, depth + 1, arg)) {
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
		*inner = 0;
		node_free(n);
	}

	return 1;
}

static struct node_walker bt_redundant = {
	0,
	0, 0,
	redundant_choice, 0,
	redundant_loop
};

void
rrd_beautify_suffixes(struct node **rrd)
{
	/*
	 * look for bits of diagram like this:
	 * -->--XYZ-->-------------------->--
	 *           |                    |
	 *           ^--<--ZYX--<--SEP--<--
	 * and replace them with the prettier equivalent:
	 * -->---XYZ--->--
	 *   |         |
	 *   ^--<--SEP--
	 */
	node_walk(rrd, &bt_collapse_suffixes, 0, 0);
}

void
rrd_beautify_botheavy(struct node **rrd)
{
	/*
	 * for loops with nothing on top and more than one thing on the bottom,
	 * flip the loop over and add a choice to skip the loop altogether.
	 * this results in a bulkier diagram, but avoids reversing the contents of
	 * the sequence.
	 */
	struct bot_heavy_context ctx = {0, 0};
	node_walk(rrd, &bt_bot_heavy, 0, &ctx);
}

void
rrd_beautify_redundant(struct node **rrd)
{
	node_walk(rrd, &bt_redundant, 0, 0);
}

void
rrd_beautify_all(struct node **rrd)
{
	rrd_beautify_suffixes(rrd);
	rrd_beautify_redundant(rrd);
	rrd_beautify_botheavy(rrd);
}

