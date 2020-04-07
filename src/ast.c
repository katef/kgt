/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "txt.h"
#include "ast.h"
#include "xalloc.h"

static int
isalphastr(const struct txt *t)
{
	size_t i;

	assert(t != NULL);
	assert(t->p != NULL);

	for (i = 0; i < t->n; i++) {
		if (isalpha((unsigned char) t->p[i])) {
			return 1;
		}
	}

	return 0;
}

struct ast_term *
ast_make_empty_term(int invisible)
{
	struct ast_term *new;

	new = xmalloc(sizeof *new);
	new->type = TYPE_EMPTY;
	new->next = NULL;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_rule_term(int invisible, struct ast_rule *rule)
{
	struct ast_term *new;

	assert(rule != NULL);

	new = xmalloc(sizeof *new);
	new->type   = TYPE_RULE;
	new->next   = NULL;
	new->u.rule = rule;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_char_term(int invisible, char c)
{
	struct ast_term *new;
	char *a;

	a = xmalloc(1); /* XXX: i don't like this */
	a[0] = c;

	new = xmalloc(sizeof *new);
	new->type        = TYPE_CS_LITERAL;
	new->next        = NULL;
	new->u.literal.p = a;
	new->u.literal.n = 1;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_literal_term(int invisible, const struct txt *literal, int ci)
{
	struct ast_term *new;

	assert(literal != NULL);
	assert(literal->p != NULL);

	new = xmalloc(sizeof *new);
	new->type      = ci ? TYPE_CI_LITERAL : TYPE_CS_LITERAL;
	new->next      = NULL;
	new->u.literal = *literal;

	/* no need for case-insensitive strings if there are no letters */
	if (!isalphastr(&new->u.literal)) {
		new->type = TYPE_CS_LITERAL;
	}

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_token_term(int invisible, const char *token)
{
	struct ast_term *new;

	assert(token != NULL);

	new = xmalloc(sizeof *new);
	new->type    = TYPE_TOKEN;
	new->next    = NULL;
	new->u.token = token;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_prose_term(int invisible, const char *prose)
{
	struct ast_term *new;

	assert(prose != NULL);

	new = xmalloc(sizeof *new);
	new->type    = TYPE_PROSE;
	new->next    = NULL;
	new->u.prose = prose;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_term *
ast_make_group_term(int invisible, struct ast_alt *group)
{
	struct ast_term *new;

	new = xmalloc(sizeof *new);
	new->type    = TYPE_GROUP;
	new->next    = NULL;
	new->u.group = group;

	new->min = 1;
	new->max = 1;

	new->invisible = invisible;

	return new;
}

struct ast_alt *
ast_make_alt(int invisible, struct ast_term *terms)
{
	struct ast_alt *new;

	new = xmalloc(sizeof *new);
	new->terms = terms;
	new->next  = NULL;

	new->invisible = invisible;

	return new;
}

struct ast_rule *
ast_make_rule(const char *name, struct ast_alt *alts)
{
	struct ast_rule *new;

	assert(name != NULL);

	new = xmalloc(sizeof *new);
	new->name = name;
	new->alts = alts;
	new->next = NULL;

	return new;
}

struct ast_rule *
ast_find_rule(const struct ast_rule *grammar, const char *name)
{
	const struct ast_rule *p;

	assert(name != NULL);

	for (p = grammar; p != NULL; p = p->next) {
		if (0 == strcmp(p->name, name)) {
			return (struct ast_rule *) p;
		}
	}

	return NULL;
}

void
ast_free_rule(struct ast_rule *rule)
{
	free(rule);
}

