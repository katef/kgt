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
#include "stack.h"
#include "pretty.h"
#include "node.h"

static int
process_loop_leaf(struct node *loop, struct stack *bp)
{
	struct node *a, *b;

	if (bp == NULL) {
		return 0;
	}

	a = loop->u.loop.backward;
	b = bp->node;

	if (node_compare(a, b)) {
		struct node *tmp;

		tmp = loop->u.loop.forward;
		loop->u.loop.forward  = loop->u.loop.backward;
		loop->u.loop.backward = tmp;

		return 1;
	}

	return 0;
}

static void
loop_switch_sides(int suflen, struct node *loop, struct stack **rl)
{
	struct node *v, **n;
	int i;

	if (suflen > 1) {
		struct node *seq;

		seq = node_create_seq(NULL);

		n = &seq->u.seq;
		node_free(loop->u.loop.forward);
		loop->u.loop.forward = seq;

		for (i = 0; i < suflen; i++) {
			v = stack_pop(rl);
			v->next = *n;
			*n = v;
		}
	} else {
		node_free(loop->u.loop.forward);
		v = stack_pop(rl);
		v->next = NULL;
		loop->u.loop.forward = v;
	}

	v = stack_pop(rl);
	if (v != NULL) {
		v->next = NULL;
	} else {
		struct node *skip;

		skip = node_create_skip();
		/*node_free(loop->u.loop.backward);*/
		loop->u.loop.backward = skip;
	}

	if (loop->u.loop.backward->type == NODE_ALT || loop->u.loop.backward->type == NODE_SEQ) {
		node_collapse(&loop->u.loop.backward);
	}
}

static int
process_loop_list(struct node *loop, struct stack *bp)
{
	struct node *list;
	struct node *p;
	struct stack *rl = NULL, *rp;
	int suffix = 0;

	list = loop->u.loop.backward;

	if (list->type == NODE_ALT) {
		return 0;
	}

	for (p = list->u.seq; p != NULL; p = p->next) {
		stack_push(&rl, p);
	}

	/* linkedlistcmp() */
	for (rp = rl; rp != NULL && bp != NULL; rp = rp->next, bp = bp->next) {
		if (!node_compare(rp->node, bp->node)) {
			break;
		}

		suffix++;
	}

	if (suffix > 0) {
		loop_switch_sides(suffix, loop, &rl);
	}

	stack_free(&rl);

	return suffix;
}

static int
process_loop(struct node *loop, struct stack *bp)
{
	if (loop->u.loop.backward->type == NODE_SKIP
	 || loop->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (loop->u.loop.backward->type == NODE_ALT || loop->u.loop.backward->type == NODE_SEQ) {
		return process_loop_list(loop, bp);
	}

	if (loop->u.loop.backward->type == NODE_LITERAL || loop->u.loop.backward->type == NODE_RULE) {
		return process_loop_leaf(loop, bp);
	}

	return 0;
}

static struct node_walker pretty_collapse_suffixes;

static int
collapse_seq(struct node *n, struct node **np, int depth, void *opaque)
{
	struct node *p, **q;
	struct stack *rl = NULL;

	for (p = n->u.seq; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->type != NODE_LOOP) {
			stack_push(&rl, p);
			continue;
		}

		suffix_len = process_loop(p, rl);

		/* delete suffix_len things from the list */
		for (i = 0; i < suffix_len; i++) {
			struct node *q;

			q = stack_pop(&rl);
			if (q == NULL) {
				return 0;
			}

			q->next = NULL;
			node_free(q);

			if (rl) {
				rl->node->next = p;
			} else {
				n->u.seq = p;
			}
		}
	}

	stack_free(&rl);

	for (q = &n->u.seq; *q != NULL; q = &(**q).next) {
		if (!node_walk(q, &pretty_collapse_suffixes, depth + 1, opaque)) {
			return 0;
		}
	}

	node_collapse(np);

	return 1;
}

static struct node_walker pretty_collapse_suffixes = {
	NULL,
	NULL, NULL,
	NULL, collapse_seq,
	NULL
};

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
void
rrd_pretty_suffixes(struct node **rrd)
{
	node_walk(rrd, &pretty_collapse_suffixes, 0, NULL);
}

