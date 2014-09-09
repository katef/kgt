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

static struct node *
transform_term(const struct ast_term *term);

static struct node *
transform_alt(const struct ast_alt *alt)
{
	struct node *list, **tail;
	const struct ast_term *p;

	list = NULL;
	tail = &list;

	for (p = alt->terms; p != NULL; p = p->next) {
		/* TODO: node_append */
		if (p->type == TYPE_EMPTY) {
			continue;
		}
		*tail = transform_term(p);
		if (*tail == NULL) {
			node_free(list);
			return NULL;
		}

		while (*tail) {
			tail = &(**tail).next;
		}
	}

	if (list == NULL) {
		return node_create_skip();
	} else if (list->next == NULL) {
		return list;
	} else {
		return node_create_sequence(list);
	}
}

static struct node *
transform_alts(const struct ast_alt *alts)
{
	struct node *list = NULL, **head = &list;
	const struct ast_alt *p;

	for (p = alts; p != NULL; p = p->next) {
		/* TODO: node_add */
		*head = transform_alt(p);
		if (*head == NULL) {
			node_free(list);
			return NULL;
		}

		while (*head) {
			head = &(**head).next;
		}
	}

	return list;
}

static struct node *
transform_empty(void)
{
	return node_create_skip();
}

static struct node *
transform_leaf(const struct ast_term *term)
{
	switch (term->type) {
	case TYPE_RULE:
		return node_create_name(term->u.rule->name);

	case TYPE_TERMINAL:
		return node_create_terminal(term->u.literal);

	default:
		errno = EINVAL;
		return NULL;
	}
}

static struct node *
transform_group(const struct ast_alt *group)
{
	struct node *list;
	struct node *alts;

	alts = transform_alts(group);
	if (alts == NULL) {
		return NULL;
	}

	list = node_create_choice(alts);

	node_collapse(&list);

	return list;
}

static struct node *
single_term(const struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:      return transform_empty();
	case TYPE_RULE:
	case TYPE_TERMINAL:   return transform_leaf (term);
	case TYPE_GROUP:      return transform_group(term->u.group);
	}

	return NULL;
}

static struct node *
optional_term(const struct ast_term *term)
{
	struct node *skip;
	struct node *n;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	skip->next = n;

	return node_create_choice(skip);
}

static struct node *
oneormore_term(const struct ast_term *term)
{
	struct node *loop;
	struct node *skip;
	struct node *n;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	loop = node_create_loop(n, skip);

	node_collapse(&loop->u.loop.forward);
	node_collapse(&loop->u.loop.backward);

	return loop;
}

static struct node *
zeroormore_term(const struct ast_term *term)
{
	struct node *skip;
	struct node *loop;
	struct node *n;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	loop = node_create_loop(skip, n);

	node_collapse(&loop->u.loop.forward);
	node_collapse(&loop->u.loop.backward);

	return loop;
}

static struct node *
transform_term(const struct ast_term *term)
{
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		struct node *(*f)(const struct ast_term *term);
	} a[] = {
		{ 1, 1, single_term     },
		{ 0, 1, optional_term   },
		{ 1, 0, oneormore_term  },
		{ 0, 0, zeroormore_term }
	};

	/* TODO: our rrd tree can't express finite term repetition */
	assert(term->max <= 1);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->max == a[i].max) {
			return a[i].f(term);
		}
	}

	return NULL;
}

struct node *
ast_to_rrd(const struct ast_rule *ast)
{
	struct node *choice;
	struct node *n;

	n = transform_alts(ast->alts);
	if (n == NULL) {
		return NULL;
	}

	choice = node_create_choice(n);

	node_collapse(&choice);

	return choice;
}

static int
node_call_walker(struct node **n, const struct node_walker *ws, int depth, void *a)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:     return ws->visit_skip     ? ws->visit_skip(*n, n, depth, a)       : -1;
	case NODE_TERMINAL: return ws->visit_terminal ? ws->visit_terminal(node, n, depth, a) : -1;
	case NODE_RULE:     return ws->visit_name     ? ws->visit_name(node, n, depth, a)     : -1;
	case NODE_CHOICE:   return ws->visit_choice   ? ws->visit_choice(node, n, depth, a)   : -1;
	case NODE_SEQUENCE: return ws->visit_sequence ? ws->visit_sequence(node, n, depth, a) : -1;
	case NODE_LOOP:     return ws->visit_loop     ? ws->visit_loop(node, n, depth, a)     : -1;
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
	case NODE_CHOICE:
		if (!node_walk_list(&node->u.choice, ws, depth + 1, a)) {
			return 0;
		}

		break;

	case NODE_SEQUENCE:
		if (!node_walk_list(&node->u.sequence, ws, depth + 1, a)) {
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
	case NODE_RULE:
	case NODE_TERMINAL:
		break;
	}

	return 1;
}
