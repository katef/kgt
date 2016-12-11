/* $Id$ */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../ast.h"
#include "../xalloc.h"

#include "rrd.h"
#include "node.h"

int
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

