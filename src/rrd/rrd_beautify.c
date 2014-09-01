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

static struct node_walker rrd_beautifier;

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
	if (bp->v->type == NT_LEAF) {
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
		struct node *nothing = node_create(NT_NOTHING);
		node_free(loop->backward);
		loop->backward = nothing;
	}

	if (loop->backward->type == NT_LIST) {
		struct node_list *back = (struct node_list *)loop->backward;
		if (!back->list->next) {
			loop->backward = back->list;
			back->list = 0;
			node_free(&back->node);
		}
	}
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
	if (loop->backward->type == NT_NOTHING
	 || loop->forward->type != NT_NOTHING)
		return 0;

	if (loop->backward->type == NT_LIST)
		return process_loop_list(loop, bp);
	if (loop->backward->type == NT_LEAF)
		return process_loop_leaf(loop, bp);

	return 0;
}

static int visit_sequence(void *arg, int depth, struct node_list *n) {
	/* look for bits of diagram like this:
	 * -->--XYZ-->-------------------->--
	 *			 |					  |
	 *			 ^--<--ZYX--<--SEP--<--
	 * and replace them with the prettier equivalent:
	 * -->---XYZ--->--
	 *	 |		   |
	 *	 ^--<--SEP--
	 */
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

	return node_walk_list(n->list, &rrd_beautifier, depth + 1, arg);
}

static struct node_walker rrd_beautifier = {
	0,
	0, 0,
	0, visit_sequence,
	0
};

void rrd_beautify(struct node *rrd) {
	node_walk(rrd, &rrd_beautifier, 0, 0);
}

