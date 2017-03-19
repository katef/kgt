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

static struct node *
transform_term(const struct ast_term *term);

static struct node *
transform_terms(const struct ast_alt *alt)
{
	struct list *list, **tail;
	const struct ast_term *p;

	list = NULL;
	tail = &list;

	for (p = alt->terms; p != NULL; p = p->next) {
		struct node *node;

		node = transform_term(p);
		if (node == NULL) {
			goto error;
		}

		list_push(tail, node);
		tail = &(*tail)->next;
	}

	return node_create_seq(list);

error:

	list_free(&list);

	return NULL;
}

static struct node *
transform_alts(const struct ast_alt *alts)
{
	struct list *list, **tail;
	const struct ast_alt *p;

	list = NULL;
	tail = &list;

	for (p = alts; p != NULL; p = p->next) {
		struct node *node;

		node = transform_terms(p);
		if (node == NULL) {
			goto error;
		}

		list_push(tail, node);
		tail = &(*tail)->next;
	}

	return node_create_alt(list);

error:

	list_free(&list);

	return NULL;
}

static struct node *
single_term(const struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:
		return node_create_skip();

	case TYPE_RULE:
		return node_create_name(term->u.rule->name);

	case TYPE_LITERAL:
		return node_create_literal(term->u.literal);

	case TYPE_TOKEN:
		return node_create_name(term->u.token);

	case TYPE_GROUP:
		return transform_alts(term->u.group);
	}
}

static struct node *
optional_term(const struct ast_term *term)
{
	struct node *skip, *n;
	struct list *list;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	list = NULL;

	list_push(&list, n);
	list_push(&list, skip);

	return node_create_alt(list);
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

	return loop;
}

static struct node *
zeroormore_term(const struct ast_term *term)
{
	struct node *skip;
	struct node *n;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	return node_create_loop(skip, n);
}

static struct node *
finite_term(const struct ast_term *term)
{
	struct node *skip;
	struct node *loop;
	struct node *n;

	n = single_term(term);
	if (n == NULL) {
		return NULL;
	}

	skip = node_create_skip();

	if (term->min > 0) {
		loop = node_create_loop(n, skip);
		loop->u.loop.min = term->min - 1;
		loop->u.loop.max = term->max - 1;
	} else {
		loop = node_create_loop(skip, n);
		loop->u.loop.min = term->min;
		loop->u.loop.max = term->max;
	}

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

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->max == a[i].max) {
			return a[i].f(term);
		}
	}

	return finite_term(term);
}

struct node *
ast_to_rrd(const struct ast_rule *ast)
{
	assert(ast != NULL);

	return transform_alts(ast->alts);
}

