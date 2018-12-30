/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * AST to Railroad transformation
 */

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

#include "../ast.h"
#include "../xalloc.h"

#include "rrd.h"
#include "node.h"
#include "list.h"

static int
transform_term(const struct ast_term *term, struct node **r);

static int
transform_terms(const struct ast_alt *alt, struct node **r)
{
	struct list *list, **tail;
	const struct ast_term *p;

	assert(r != NULL);

	list = NULL;
	tail = &list;

	for (p = alt->terms; p != NULL; p = p->next) {
		struct node *node;

		if (!transform_term(p, &node)) {
			goto error;
		}

		list_push(tail, node);
		tail = &(*tail)->next;
	}

	*r = node_create_seq(list);

	return 1;

error:

	list_free(&list);

	return 0;
}

static int
transform_alts(const struct ast_alt *alts, struct node **r)
{
	struct list *list, **tail;
	const struct ast_alt *p;

	assert(r != NULL);

	list = NULL;
	tail = &list;

	for (p = alts; p != NULL; p = p->next) {
		struct node *node;

		if (!transform_terms(p, &node)) {
			goto error;
		}

		list_push(tail, node);
		tail = &(*tail)->next;
	}

	*r = node_create_alt(list);

	return 1;

error:

	list_free(&list);

	return 0;
}

static int
single_term(const struct ast_term *term, struct node **r)
{
	assert(r != NULL);

	switch (term->type) {
	case TYPE_EMPTY:
		*r = NULL;
		return 1;

	case TYPE_RULE:
		*r = node_create_name(term->u.rule->name);
		return 1;

	case TYPE_LITERAL:
		*r = node_create_literal(term->u.literal);
		return 1;

	case TYPE_TOKEN:
		*r = node_create_name(term->u.token);
		return 1;

	case TYPE_GROUP:
		return transform_alts(term->u.group, r);
	}
}

static int
optional_term(const struct ast_term *term, struct node **r)
{
	struct node *n;
	struct list *list;

	assert(r != NULL);

	if (!single_term(term, &n)) {
		return 0;
	}

	list = NULL;

	list_push(&list, n);

	*r = node_create_alt_skippable(list);

	return 1;
}

static int
oneormore_term(const struct ast_term *term, struct node **r)
{
	struct node *n;

	assert(r != NULL);

	if (!single_term(term, &n)) {
		return 0;
	}

	*r = node_create_loop(n, NULL);

	return 1;
}

static int
zeroormore_term(const struct ast_term *term, struct node **r)
{
	struct node *n;

	assert(r != NULL);

	if (!single_term(term, &n)) {
		return 0;
	}

	*r = node_create_loop(NULL, n);

	return 1;
}

static int
finite_term(const struct ast_term *term, struct node **r)
{
	struct node *loop;
	struct node *n;

	assert(r != NULL);

	if (!single_term(term, &n)) {
		return 0;
	}

	if (term->min > 0) {
		loop = node_create_loop(n, NULL);
		loop->u.loop.min = term->min - 1;
		loop->u.loop.max = term->max - 1;
	} else {
		loop = node_create_loop(NULL, n);
		loop->u.loop.min = term->min;
		loop->u.loop.max = term->max;
	}

	*r = loop;

	return 1;
}

static int
transform_term(const struct ast_term *term, struct node **r)
{
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		int (*f)(const struct ast_term *term, struct node **r);
	} a[] = {
		{ 1, 1, single_term     },
		{ 0, 1, optional_term   },
		{ 1, 0, oneormore_term  },
		{ 0, 0, zeroormore_term }
	};

	assert(r != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->max == a[i].max) {
			return a[i].f(term, r);
		}
	}

	return finite_term(term, r);
}

int
ast_to_rrd(const struct ast_rule *ast, struct node **r)
{
	assert(ast != NULL);
	assert(r != NULL);

	return transform_alts(ast->alts, r);
}

