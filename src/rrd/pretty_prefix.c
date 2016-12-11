#include <assert.h>
#include <stddef.h>

#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"
#include "node.h"

static int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *opaque);

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

static struct node_walker pretty_collapse_prefixes;

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
		if (!node_walk(q, &pretty_collapse_prefixes, depth + 1, opaque)) {
			return 0;
		}
	}

	node_collapse(np);

	return 1;
}

static struct node_walker pretty_collapse_prefixes = {
	NULL,
	NULL, NULL,
	NULL, collapse_seq,
	NULL
};

static int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *opaque)
{
	int (*f)(struct node *, struct node **, int, void *);
	struct node *node;

	assert(n != NULL);
	assert(ws != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:    f = ws->visit_skip;    break;
	case NODE_LITERAL: f = ws->visit_literal; break;
	case NODE_RULE:    f = ws->visit_name;    break;
	case NODE_ALT:     f = ws->visit_alt;     break;
	case NODE_SEQ:     f = ws->visit_seq;     break;
	case NODE_LOOP:    f = ws->visit_loop;    break;
	}

	if (f != NULL) {
		return f(node, n, depth, opaque);
	}

	switch (node->type) {
		struct node **p;

	case NODE_ALT:
		for (p = &node->u.alt; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, ws, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_SEQ:
		for (p = &node->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, ws, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_LOOP:
		if (!node_walk(&node->u.loop.forward, ws, depth + 1, opaque)) {
			return 0;
		}

		if (!node_walk(&node->u.loop.backward, ws, depth + 1, opaque)) {
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
	node_walk(rrd, &pretty_collapse_prefixes, 0, NULL);
}

