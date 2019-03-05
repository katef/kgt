/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * AST node rewriting
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "txt.h"
#include "ast.h"
#include "rewrite.h"
#include "xalloc.h"

static void
add_alt(struct ast_alt **alt, const struct txt *t)
{
	struct ast_term *term;
	struct ast_alt *new;
	struct txt q;

	assert(alt != NULL);
	assert(t != NULL);
	assert(t->p != NULL);

	/* TODO: move ownership to ast_make_*() and no need to make a new struct txt here */
	q = xtxtdup(t);

	term = ast_make_literal_term(&q, 0);

	new = ast_make_alt(term);
	new->next = *alt;
	*alt = new;
}

static void
f(struct ast_alt **alt, const struct txt *t, char *p, size_t n)
{
	assert(alt != NULL);
	assert(t != NULL);
	assert(t->p != NULL);
	assert(p != NULL);

	if (n == 0) {
		add_alt(alt, t);
		return;
	}

	if (!isalpha((unsigned char) *p)) {
		f(alt, t, p + 1, n - 1);
		return;
	}

	*p = toupper((unsigned char) *p);
	f(alt, t, p + 1, n - 1);
	*p = tolower((unsigned char) *p);
	f(alt, t, p + 1, n - 1);
}

static void
rewrite_ci(struct ast_term *term)
{
	size_t i;

	assert(term->type == TYPE_CI_LITERAL);

	/* case is normalised during AST creation */
	for (i = 0; i < term->u.literal.n; i++) {
		if (!isalpha((unsigned char) term->u.literal.p[i])) {
			continue;
		}

		assert(islower((unsigned char) term->u.literal.p[i]));
	}

	/* we repurpose the existing node, which breaks abstraction for freeing */
	term->type = TYPE_GROUP;
	term->u.group = NULL;

	f(&term->u.group, &term->u.literal, (void *) term->u.literal.p, term->u.literal.n);

	free((void *) term->u.literal.p);
}

static void
walk_alt(struct ast_alt *alt)
{
	struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		switch (term->type) {
		case TYPE_EMPTY:
		case TYPE_CS_LITERAL:
		case TYPE_TOKEN:
		case TYPE_PROSE:
			break;

		case TYPE_GROUP:
			walk_alt(term->u.group);
			break;

		case TYPE_RULE:
			/* (struct ast_term).u.rule is just for the name; don't recurr into it */
			break;

		case TYPE_CI_LITERAL:
			rewrite_ci(term);
			break;
		}
	}
}

void
rewrite_ci_literals(struct ast_rule *grammar)
{
	struct ast_rule *rule;
	struct ast_alt *alt;

	for (rule = grammar; rule != NULL; rule = rule->next) {
		for (alt = rule->alts; alt != NULL; alt = alt->next) {
			walk_alt(alt);
		}
	}
}

