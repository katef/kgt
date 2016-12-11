#include <assert.h>
#include <stddef.h>

#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"
#include "node.h"

static int
node_walk(struct node **n, int depth, void *opaque);

static int
process_loop(struct node *loop) {
	int suflen = 0;
	struct node *a, *b, *seq = NULL, **tail = &seq;

	if (loop->next == NULL || loop->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (loop->u.loop.backward->type != NODE_SEQ) {
		struct node *tmp;
		if (!node_compare(loop->u.loop.backward, loop->next)) {
			return 0;
		}

		tmp = loop->u.loop.forward;
		loop->u.loop.forward  = loop->u.loop.backward;
		loop->u.loop.backward = tmp;

		return 1;
	}

	for (a = loop->u.loop.backward->u.seq, b = loop->next; a != NULL && b != NULL; a = a->next, b = b->next) {
		if (!node_compare(a, b)) {
			break;
		}
		*tail = a;
		tail = &a->next;
		suflen++;
	}

	if (suflen > 0) {
		loop->u.loop.backward = *tail;
		*tail = NULL;
		node_free(loop->u.loop.forward);
		loop->u.loop.forward = node_create_seq(seq);
	}

	return suflen;
}

static int
collapse_seq(struct node *n, struct node **np, int depth, void *opaque)
{
	struct node *p, **q;

	for (p = n->u.seq; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->type != NODE_LOOP) {
			continue;
		}

		suffix_len = process_loop(p);

		for (i = 0; i < suffix_len; i++) {
			struct node *q = p->next;
			p->next = q->next;
			q->next = NULL;
			node_free(q);
		}
	}

	for (q = &n->u.seq; *q != NULL; q = &(**q).next) {
		if (!node_walk(q, depth + 1, opaque)) {
			return 0;
		}
	}

	node_collapse(np);

	return 1;
}

static int
node_walk(struct node **n, int depth, void *opaque)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
		struct node **p;

	case NODE_SEQ:
		return collapse_seq(node, n, depth, opaque);

	case NODE_ALT:
		for (p = &node->u.alt; *p != NULL; p = &(**p).next) {
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

void
rrd_pretty_prefixes(struct node **rrd)
{
	node_walk(rrd, 0, NULL);
}

