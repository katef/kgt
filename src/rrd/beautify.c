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

/* quick linked list - we need it to keep a backwards list of the nodes we've encountered */
struct bnode {
	struct node *v;
	struct bnode *next;
};

static void b_push(struct bnode **list, struct node *v) {
	struct bnode bn;
	bn.v = v;
	bn.next = *list;
	*list = xmalloc(sizeof bn);
	**list = bn;
}

static int b_pop(struct bnode **list, struct node **out) {
	struct bnode *n;
	if (!list || !*list)
		return 0;
	n = *list;
	*list = (**list).next;
	if (out)
		*out = n->v;
	free(n);
	return 1;
}

static void b_clear(struct bnode **list) {
	while (*list)
		b_pop(list, 0);
}

static int leaves_eq(struct node_leaf *a, struct node_leaf *b) {
	return a->type == b->type && !strcmp(a->text, b->text);
}

static int process_loop_leaf(struct node_loop *loop, struct bnode *bp) {
	struct node_leaf *a = (struct node_leaf *)loop->backward;
	if (bp && bp->v->type == NT_LEAF) {
		struct node_leaf *b = (struct node_leaf *)bp->v;
		if (leaves_eq(a, b)) {
			struct node *tmp = loop->forward;
			loop->forward = loop->backward;
			loop->backward = tmp;
			return 1;
		}
	}
	return 0;
}

static void loop_switch_sides(int suflen, struct node_loop *loop, struct bnode **rl) {
	int i;
	struct node *v, **n;

	if (suflen > 1) {
		struct node_list *seq = node_create(NT_LIST);
		seq->type = LIST_SEQUENCE;
		seq->list = 0;
		n = &seq->list;
		node_free(loop->forward);
		loop->forward = &seq->node;

		for(i = 0; i < suflen; i++) {
			b_pop(rl, &v);
			v->next = *n;
			*n = v;
		}
	} else {
		node_free(loop->forward);
		b_pop(rl, &v);
		v->next = 0;
		loop->forward = v;
	}

	if (b_pop(rl, &v)) {
		v->next = 0;
	} else {
		struct node *nothing = node_create(NT_SKIP);
		node_free(loop->backward);
		loop->backward = nothing;
	}

	if (loop->backward->type == NT_LIST)
		node_collapse(&loop->backward);
}

static int process_loop_list(struct node_loop *loop, struct bnode *bp) {
	struct node_list *list = (struct node_list *)loop->backward;
	struct node *p;
	struct bnode *rl = 0, *rp;
	int suffix = 0;
	if (list->type == LIST_CHOICE)
		return 0;
	for (p = list->list; p; p = p->next)
		b_push(&rl, p);
	rp = rl;
	/* linkedlistcmp() */
	while (rp && bp) {
		struct node_leaf *a, *b;
		if (rp->v->type != NT_LEAF || bp->v->type != NT_LEAF)
			break;
		a = (struct node_leaf *)rp->v;
		b = (struct node_leaf *)bp->v;
		if (!leaves_eq(a, b))
			break;
		suffix++;
		rp = rp->next;
		bp = bp->next;
	}
	if (suffix > 0)
		loop_switch_sides(suffix, loop, &rl);
	b_clear(&rl);
	return suffix;
}

static int process_loop(struct node_loop *loop, struct bnode *bp) {
	if (loop->backward->type == NT_SKIP
	 || loop->forward->type != NT_SKIP)
		return 0;

	if (loop->backward->type == NT_LIST)
		return process_loop_list(loop, bp);
	if (loop->backward->type == NT_LEAF)
		return process_loop_leaf(loop, bp);

	return 0;
}

static struct node_walker bt_collapse_suffixes;

static int collapse_list(struct node_list *n, struct node **np, int depth, void *arg) {
	struct node *p;
	struct bnode *rl = 0;

	for (p = n->list; p; p = p->next) {
		if (p->type == NT_LOOP) {
			struct node_loop *loop = (struct node_loop *)p;
			int i, suffix_len;
			suffix_len = process_loop(loop, rl);
			/* delete suffix_len things from the list */
			for (i = 0; i < suffix_len; i++) {
				struct node *q;
				if (!b_pop(&rl, &q))
					return 0;
				node_free(q);
				if (rl) {
					rl->v->next = p;
				} else {
					n->list = p;
				}
			}
		} else {
			b_push(&rl, p);
		}
	}

	b_clear(&rl);

	if (!node_walk_list(&n->list, &bt_collapse_suffixes, depth + 1, arg))
		return 0;

	node_collapse(np);
	return 1;
}

static struct node_walker bt_collapse_suffixes = {
	0,
	0, 0,
	0, collapse_list,
	0
};

static struct node_walker bt_bot_heavy;

struct bot_heavy_context {
	int applied;
	int everything;
};

static int bot_heavy_sequence(struct node_list *n, struct node **np, int depth, void *arg) {
	struct bot_heavy_context *ctx = arg;
	struct node **p;
	int anything = 0;

	for (p = &n->list; *p; p = &(**p).next) {
		ctx->applied = 0;
		if (!node_walk(p, &bt_bot_heavy, depth + 1, ctx))
			return 0;
		anything = anything || ctx->applied;
	}

	if (0 && anything) {
		ctx->everything = 1;
		node_walk_list(&n->list, &bt_bot_heavy, depth + 1, ctx);
		ctx->everything = 0;
	}

	return 1;
}

static int bot_heavy_loop(struct node_loop *n, struct node **np, int depth, void *arg) {
	struct bot_heavy_context *ctx = arg;
	int everything = ctx->everything;
	ctx->everything = 0;
	do {
		struct node_list *choice;
		struct node *tmp, *nothing;
		if (n->forward->type != NT_SKIP)
			break;
		if (n->backward->type == NT_SKIP
		 || n->backward->type == NT_LOOP)
			break;
		if (!everything && n->backward->type == NT_LEAF)
			break;
		if (n->backward->type == NT_LIST) {
			struct node_list *list = (struct node_list *)n->backward;
			if (list->type == LIST_CHOICE) {
				struct node *p;
				int c = 0;
				for (p = list->list; p; p = p->next) {
					if (p->type == NT_LIST || p->type == NT_LOOP)
						c = 1;
				}
				if (!c)
					break;
			}
		}
		tmp = n->backward;
		n->backward = n->forward;
		n->forward = tmp;
		/* short-circuit */
		choice = node_create(NT_LIST);
		choice->type = LIST_CHOICE;
		nothing = node_create(NT_SKIP);
		nothing->next = &n->node;
		choice->list = nothing;
		choice->node.next = n->node.next;
		n->node.next = 0;
		*np = &choice->node;
		if (!node_walk(&nothing->next, &bt_bot_heavy, depth + 1, ctx))
			return 0;
		ctx->applied = 1;
		ctx->everything = everything;
		return 1;
	} while (0);

	if (!node_walk(&n->forward, &bt_bot_heavy, depth + 1, ctx))
		return 0;
	if (!node_walk(&n->backward, &bt_bot_heavy, depth + 1, ctx))
		return 0;

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

static int redundant_choice(struct node_list *n, struct node **np, int depth, void *arg) {
	int nc = 0, isopt = 0;
	struct node **p, **loop = 0;
	for (p = &n->list; *p; p = &(**p).next) {
		nc++;
		if (!node_walk(p, &bt_redundant, depth + 1, arg))
			return 0;
		if ((**p).type == NT_SKIP) {
			isopt = 1;
		}
		if ((**p).type == NT_LOOP) {
			loop = p;
		}
	}

	if (nc == 2 && isopt && loop) {
		struct node_loop *l = (struct node_loop *)*loop;
		/* special case: if an optional loop has an empty half, we can elide the NT_CHOICE */
		if (l->forward->type == NT_SKIP) {
			*np = *loop;
			*loop = 0;
			node_free(&n->node);
		} else if (l->backward->type == NT_SKIP) {
			struct node *tmp = l->backward;
			l->backward = l->forward;
			l->forward = tmp;
			*np = *loop;
			*loop = 0;
			node_free(&n->node);
		}
	} else {
		int i = 0;
		/* fold nested choices into this one */
		for (p = &n->list; *p; p = &(**p).next) {
			struct node_list *choice;
			struct node *last;
			i++;
			if ((**p).type != NT_LIST)
				continue;
			choice = (struct node_list *)*p;
			if (choice->type != LIST_CHOICE)
				continue;
			for (last = choice->list; last->next; last = last->next) ;
			last->next = (**p).next;
			*p = choice->list;
			choice->list = 0;
			node_free(&choice->node);
		}
	}

	return 1;
}

static int redundant_loop(struct node_loop *n, struct node **np, int depth, void *arg) {
	struct node **inner = 0;
	struct node_loop *loop;

	if (!node_walk(&n->forward, &bt_redundant, depth + 1, arg))
		return 0;
	if (!node_walk(&n->backward, &bt_redundant, depth + 1, arg))
		return 0;

	if (n->forward->type == NT_LOOP && n->backward->type == NT_SKIP) {
		loop = (struct node_loop *)n->forward;
		if (loop->forward->type == NT_SKIP || loop->backward->type == NT_SKIP)
			inner = &n->forward;
	} else if (n->backward->type == NT_LOOP && n->forward->type == NT_SKIP) {
		loop = (struct node_loop *)n->backward;
		if (loop->forward->type == NT_SKIP) {
			inner = &n->backward;
		} else if (loop->backward->type == NT_SKIP) {
			struct node *tmp = loop->backward;
			loop->backward = loop->forward;
			loop->forward = tmp;
			inner = &n->backward;
		}
	}

	if (inner) {
		*np = *inner;
		*inner = 0;
		node_free(&n->node);
	}

	return 1;
}

static struct node_walker bt_redundant = {
	0,
	0, 0,
	redundant_choice, 0,
	redundant_loop
};

void rrd_beautify_suffixes(struct node **rrd) {
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

void rrd_beautify_botheavy(struct node **rrd) {
	/*
	 * for loops with nothing on top and more than one thing on the bottom,
	 * flip the loop over and add a choice to skip the loop altogether.
	 * this results in a bulkier diagram, but avoids reversing the contents of
	 * the sequence.
	 */
	struct bot_heavy_context ctx = {0, 0};
	node_walk(rrd, &bt_bot_heavy, 0, &ctx);
}

void rrd_beautify_redundant(struct node **rrd) {
	node_walk(rrd, &bt_redundant, 0, 0);
}

void rrd_beautify_all(struct node **rrd) {
	rrd_beautify_suffixes(rrd);
	rrd_beautify_redundant(rrd);
	rrd_beautify_botheavy(rrd);
}

