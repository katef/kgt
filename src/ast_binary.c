/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>
#include <ctype.h>

#include "txt.h"
#include "ast.h"

static int
walk_term(const struct ast_term *term);

static int
walk_alts(const struct ast_alt *alts)
{
	const struct ast_alt *p;
	const struct ast_term *q;

	for (p = alts; p != NULL; p = p->next) {
		for (q = p->terms; q != NULL; q = q->next) {
			if (walk_term(q)) {
				return 1;
			}
		}
	}

	return 0;
}

static int
binary_literal(const struct txt *t)
{
	size_t i;

	assert(t != NULL);

	for (i = 0; i < t->n; i++) {
		if (!isprint((unsigned char) t->p[i])) {
			return 1;
		}
	}

	return 0;
}

static int
walk_term(const struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:
	case TYPE_RULE:
	case TYPE_TOKEN:
	case TYPE_PROSE:
		return 0;

	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
		return binary_literal(&term->u.literal);

	case TYPE_GROUP:
		return walk_alts(term->u.group);
	}

	assert(!"unreached");
}

int
ast_binary(const struct ast_rule *ast)
{
	return walk_alts(ast->alts);
}

