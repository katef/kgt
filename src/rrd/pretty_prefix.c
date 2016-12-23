#include <assert.h>
#include <stddef.h>

#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static int
process_loop(struct node *loop, struct list *next)
{
	int suflen;
	struct list *a, *b, *seq = NULL, **tail = &seq;

	assert(loop != NULL);
	assert(loop->type == NODE_LOOP);

	if (next == NULL || loop->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (loop->u.loop.backward->type != NODE_SEQ) {
		struct node *tmp;

		if (!node_compare(loop->u.loop.backward, next->node)) {
			return 0;
		}

		tmp = loop->u.loop.forward;
		loop->u.loop.forward  = loop->u.loop.backward;
		loop->u.loop.backward = tmp;

		return 1;
	}

	suflen = 0;

	for (a = loop->u.loop.backward->u.seq, b = next; a != NULL && b != NULL; a = a->next, b = b->next) {
		if (!node_compare(a->node, b->node)) {
			break;
		}

		*tail = a;
		tail = &a->next;
		suflen++;
	}

	if (suflen > 0) {
		loop->u.loop.backward = (*tail)->node;
		node_free(loop->u.loop.forward);
		loop->u.loop.forward = node_create_seq(seq);
	}

	return suflen;
}

static void
collapse_seq(struct node *n)
{
	struct list *p;

	for (p = n->u.seq; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->node->type != NODE_LOOP) {
			continue;
		}

		/* TODO: instead of finding the suffix length and then collapsing,
		 * i'd rather collapse one node at a time as we go */

		suffix_len = process_loop(p->node, p->next);

		for (i = 0; i < suffix_len; i++) {
			struct list *t = p->next;
			p->next = t->next;
			t->next = NULL;
			node_free(t->node);
			list_free(&t);
		}
	}
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

void
rrd_pretty_prefixes(struct node **rrd)
{
	node_walk(rrd);
}

