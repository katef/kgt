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
#include "list.h"

static int
process_loop_leaf(struct node *loop, struct stack *bp)
{
	struct node *a, *b;
	struct node *tmp;

	assert(loop != NULL);

	if (bp == NULL) {
		return 0;
	}

	a = loop->u.loop.backward;
	b = bp->node;

	if (!node_compare(a, b)) {
		return 0;
	}

	tmp = loop->u.loop.forward;
	loop->u.loop.forward  = loop->u.loop.backward;
	loop->u.loop.backward = tmp;

	return 1;
}

static void
loop_switch_sides(int suflen, struct node *loop, struct stack **rl)
{
	struct list *v;
	int i;

	if (suflen > 1) {
		struct list *list;

		list = NULL;

		for (i = 0; i < suflen; i++) {
			list_push(&list, stack_pop(rl));
		}

		node_free(loop->u.loop.forward);
		loop->u.loop.forward = node_create_seq(list);
	} else {
		node_free(loop->u.loop.forward);

		loop->u.loop.forward = stack_pop(rl);
	}

	v = xmalloc(sizeof *v);

	v->node = stack_pop(rl);
	if (v != NULL) {
		v->next = NULL;
	} else {
		struct node *skip;

/* XXX: v->next? */

		skip = node_create_skip();
		/*node_free(loop->u.loop.backward);*/
		loop->u.loop.backward = skip;
	}

/* XXX: where does v get output? */
}

static int
process_loop_list(struct node *loop, struct stack *bp)
{
	struct node *list;
	struct list *p;
	struct stack *rl = NULL, *rp;
	int suffix = 0;

	list = loop->u.loop.backward;

	if (list->type == NODE_ALT) {
		return 0;
	}

	for (p = list->u.seq; p != NULL; p = p->next) {
		stack_push(&rl, p->node);
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

static void
collapse_seq(struct node *n)
{
	struct list *p;
	struct stack *rl;

	rl = NULL;

	for (p = n->u.seq; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->node->type != NODE_LOOP) {
			stack_push(&rl, p->node);
			continue;
		}

		/* TODO: instead of finding the suffix length and then collapsing,
		 * i'd rather collapse one node at a time as we go */

		suffix_len = process_loop(p->node, rl);

		/* delete suffix_len things from the list */
		for (i = 0; i < suffix_len; i++) {
			struct node *t;
			struct list *new;

			t = stack_pop(&rl);
			assert(t != NULL);
			if (t == NULL) {
				return;
			}

			node_free(t);

/* XXX:
			if (rl) {
				rl->node->next = t;
??? */

			new = NULL;

			list_push(&new, stack_pop(&rl));

			n->u.seq = new;
		}
	}

	stack_free(&rl);
}

static int
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
		collapse_seq(*n);

		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(&(*p)->node);
		}

		break;

	case NODE_LOOP:
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
	node_walk(rrd);
}

