/* $Id$ */

/*
 * AST to Railroad transformation
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../ast.h"
#include "../xalloc.h"

#include "rrd.h"
#include "node.h"

/* XXX: perhaps belongs under beautify */
void
node_collapse(struct node **n)
{
	struct node *list;

	if ((**n).type != NODE_LIST) {
		return;
	}

	list = *n;

	if (list->u.list.list == NULL || list->u.list.list->next == NULL) {
		return;
	}

	*n = list->u.list.list;
	list->u.list.list = NULL;

	node_free(list);

	node_collapse(n);
}

static int transform_alts(struct node **, struct ast_alt *);
static int transform_alt(struct node **, struct ast_alt *);
static int transform_term(struct node **, struct ast_term *);
static int transform_empty(struct node **);
static int transform_leaf(struct node **, struct ast_term *);
static int transform_group(struct node **, struct ast_alt *);

static int
transform_alts(struct node **on, struct ast_alt *alts)
{
	struct node *list = 0, **head = &list;
	struct ast_alt *p;

	for (p = alts; p != NULL; p = p->next) {
		if (!transform_alt(head, p)) {
			return 0;
		}

		while (*head) {
			head = &(**head).next;
		}
	}

	*on = list;

	return 1;
}

static int
transform_alt(struct node **on, struct ast_alt *alt)
{
	struct node *list, **tail;
	struct ast_term *p;

	list = NULL;
	tail = &list;

	for (p = alt->terms; p != NULL; p = p->next) {
		if (!transform_term(tail, p)) {
			return 0;
		}

		while (*tail) {
			tail = &(**tail).next;
		}
	}

	if (list->next == NULL) {
		*on = list;
	} else {
		*on = node_create_list(LIST_SEQUENCE, list);
	}

	return 1;
}

static int
single_term(struct node **on, struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:      return transform_empty(on);
	case TYPE_PRODUCTION:
	case TYPE_TERMINAL:   return transform_leaf (on, term);
	case TYPE_GROUP:      return transform_group(on, term->u.group);
	}

	return 0;
}

static int
optional_term(struct node **on, struct ast_term *term)
{
	struct node *skip;

	skip = node_create_skip();

	*on = node_create_list(LIST_CHOICE, skip);

	if (!single_term(&skip->next, term)) {
		return 0;
	}

	return 1;
}

static int
oneormore_term(struct node **on, struct ast_term *term)
{
	struct node *skip;
	struct node *choice;
	struct node *loop;

	skip = node_create_skip();

	choice = node_create_list(LIST_CHOICE, NULL);

	loop = node_create_loop(choice, skip);
	*on = loop;

	if (!single_term(&choice->u.list.list, term)) {
		return 0;
	}

	node_collapse(&loop->u.loop.forward);
	node_collapse(&loop->u.loop.backward);

	return 1;
}

static int
zeroormore_term(struct node **on, struct ast_term *term)
{
	struct node *skip;
	struct node *choice;
	struct node *loop;

	skip = node_create_skip();

	choice = node_create_list(LIST_CHOICE, NULL);

	loop = node_create_loop(skip, choice);
	*on = loop;

	if (!single_term(&skip->next, term)) {
		return 0;
	}

	node_collapse(&loop->u.loop.forward);
	node_collapse(&loop->u.loop.backward);

	return 1;
}

static int
transform_term(struct node **on, struct ast_term *term)
{
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		int (*f)(struct node **on, struct ast_term *term);
	} a[] = {
		{ 1, 1, single_term     },
		{ 0, 1, optional_term   },
		{ 1, 0, oneormore_term  },
		{ 0, 0, zeroormore_term }
	};

	/* TODO: our rrd tree can't express finite term repetition */
	assert(term->max <= 1);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->min == a[i].min) {
			return a[i].f(on, term);
		}
	}

	return 0;
}

static int
transform_empty(struct node **on)
{
	*on = node_create_skip();
	return 1;
}

static int
transform_leaf(struct node **on, struct ast_term *term)
{
	switch (term->type) {
	case TYPE_PRODUCTION:
		*on = node_create_leaf(LEAF_IDENTIFIER, term->u.name);
		break;

	case TYPE_TERMINAL:
		*on = node_create_leaf(LEAF_TERMINAL, term->u.literal);
		break;

	default:
		errno = EINVAL;
		return 0;
	}

	return 1;
}

static int
transform_group(struct node **on, struct ast_alt *group)
{
	struct node *list;

	list = node_create_list(LIST_SEQUENCE, NULL);

	*on = list;

	if (!transform_alts(&list->u.list.list, group)) {
		return 0;
	}

	node_collapse(on);

	return 1;
}

int
ast_to_rrd(struct ast_production *ast, struct node **rrd)
{
	struct node *choice;

	choice = node_create_list(LIST_CHOICE, NULL);

	if (!transform_alts(&choice->u.list.list, ast->alts)) {
		return 0;
	}

	*rrd = choice;

	node_collapse(rrd);

	return 1;
}

static int
node_call_walker(struct node **n, const struct node_walker *ws, int depth, void *a)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:
		return ws->visit_skip ? ws->visit_skip(*n, n, depth, a) : -1;

	case NODE_LEAF:
		if (node->u.leaf.type == LEAF_IDENTIFIER) {
			return ws->visit_identifier ? ws->visit_identifier(node, n, depth, a) : -1;
		} else {
			return ws->visit_terminal   ? ws->visit_terminal(node, n, depth, a)   : -1;
		}

	case NODE_LIST:
		if (node->u.list.type == LIST_CHOICE) {
			return ws->visit_choice   ? ws->visit_choice(node, n, depth, a)   : -1;
		} else {
			return ws->visit_sequence ? ws->visit_sequence(node, n, depth, a) : -1;
		}

	case NODE_LOOP:
		return ws->visit_loop ? ws->visit_loop(node, n, depth, a) : -1;
	}

	return -1;
}

int
node_walk_list(struct node **n, const struct node_walker *ws, int depth, void *a)
{
	for (; *n != NULL; n = &(**n).next) {
		if (!node_walk(n, ws, depth, a)) {
			return 0;
		}
	}

	return 1;
}

int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *a)
{
	struct node *node;
	int r;

	assert(n != NULL);

	node = *n;

	r = node_call_walker(n, ws, depth, a);
	if (r == 0) {
		return 0;
	}

	if (r != -1) {
		return 1;
	}

	switch (node->type) {
	case NODE_LIST:
		if (!node_walk_list(&node->u.list.list, ws, depth + 1, a)) {
			return 0;
		}

		break;

	case NODE_LOOP:
		if (!node_walk(&node->u.loop.forward, ws, depth + 1, a)) {
			return 0;
		}

		if (!node_walk(&node->u.loop.backward, ws, depth + 1, a)) {
			return 0;
		}

		break;

	case NODE_SKIP:
	case NODE_LEAF:
		break;
	}

	return 1;
}
