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
walk_alts(const struct ast_alt *alts);

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
	assert(term != NULL);

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

static int
walk_alts(const struct ast_alt *alts)
{
	const struct ast_alt *alt;
	const struct ast_term *term;

	for (alt = alts; alt != NULL; alt = alt->next) {
		for (term = alt->terms; term != NULL; term = term->next) {
			if (walk_term(term)) {
				return 1;
			}
		}
	}

	return 0;
}

int
ast_binary(const struct ast_rule *grammar)
{
	const struct ast_rule *rule;

	for (rule = grammar; rule != NULL; rule = rule->next) {
		if (walk_alts(rule->alts)) {
			return 1;
		}
	}

	return 0;
}

