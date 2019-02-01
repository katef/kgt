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
#include <ctype.h>

#include "ast.h"
#include "rewrite.h"
#include "xalloc.h"

static void
add_alt(struct ast_alt **alt, const char *s)
{
	struct ast_term *term;
	struct ast_alt *new;

	assert(alt != NULL);

	s = xstrdup(s);

	term = ast_make_literal_term(s, 0);

	new = ast_make_alt(term);
	new->next = *alt;
	*alt = new;
}

static void
f(struct ast_alt **alt, char *s, char *p)
{
	assert(alt != NULL);
	assert(s != NULL);

	if (*p == '\0') {
		add_alt(alt, s);
		return;
	}

	if (!isalpha((unsigned char) *p)) {
		f(alt, s, p + 1);
		return;
	}

	*p = toupper((unsigned char) *p);
	f(alt, s, p + 1);
	*p = tolower((unsigned char) *p);
	f(alt, s, p + 1);
}

static void
rewrite_ci(struct ast_term *term)
{
	char *s, *p;

	assert(term->type == TYPE_CI_LITERAL);

	s = (void *) term->u.literal;

	/* case is normalised during AST creation */
	for (p = s; *p != '\0'; p++) {
		if (!isalpha((unsigned char) *p)) {
			continue;
		}

		assert(islower((unsigned char) *p));
	}

	/* we repurpose the existing node, which breaks abstraction for freeing */
	term->type = TYPE_GROUP;
	term->u.group = NULL;

	f(&term->u.group, s, s);

	free(s);
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
			break;

		case TYPE_GROUP:
			walk_alt(term->u.group);
			break;

		case TYPE_RULE:
			rewrite_ci_literals(term->u.rule);
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

