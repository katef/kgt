/* $Id$ */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../ast.h"
#include "../xalloc.h"

#include "rrd.h"
#include "node.h"

static int
node_call_walker(struct node **n, const struct node_walker *ws, int depth, void *a)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:    return ws->visit_skip    ? ws->visit_skip(*n, n, depth, a)      : -1;
	case NODE_LITERAL: return ws->visit_literal ? ws->visit_literal(node, n, depth, a) : -1;
	case NODE_RULE:    return ws->visit_name    ? ws->visit_name(node, n, depth, a)    : -1;
	case NODE_ALT:     return ws->visit_alt     ? ws->visit_alt(node, n, depth, a)     : -1;
	case NODE_SEQ:     return ws->visit_seq     ? ws->visit_seq(node, n, depth, a)     : -1;
	case NODE_LOOP:    return ws->visit_loop    ? ws->visit_loop(node, n, depth, a)    : -1;
	}

	return -1;
}

int
node_walk_list(struct node **n, const struct node_walker *ws, int depth, void *opaque)
{
	for (; *n != NULL; n = &(**n).next) {
		if (!node_walk(n, ws, depth, opaque)) {
			return 0;
		}
	}

	return 1;
}

int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *opaque)
{
	struct node *node;
	int r;

	assert(n != NULL);

	node = *n;

	r = node_call_walker(n, ws, depth, opaque);
	if (r == 0) {
		return 0;
	}

	if (r != -1) {
		return 1;
	}

	switch (node->type) {
	case NODE_ALT:
		if (!node_walk_list(&node->u.alt, ws, depth + 1, opaque)) {
			return 0;
		}

		break;

	case NODE_SEQ:
		if (!node_walk_list(&node->u.seq, ws, depth + 1, opaque)) {
			return 0;
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

