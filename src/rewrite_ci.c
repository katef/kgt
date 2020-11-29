/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * AST node rewriting
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "txt.h"
#include "ast.h"
#include "rewrite.h"
#include "xalloc.h"
#include "compiler_specific.h"

static int
walk_alts(struct ast_alt *alts);

static void
add_alt(int invisible, struct ast_alt **alt, const struct txt *t)
{
	struct ast_term *term;
	struct ast_alt *new;
	struct txt q;

	assert(alt != NULL);
	assert(t != NULL);
	assert(t->p != NULL);

	/* TODO: move ownership to ast_make_*() and no need to make a new struct txt here */
	q = xtxtdup(t);

	term = ast_make_literal_term(invisible, &q, 0);

	new = ast_make_alt(invisible, term);
	new->next = *alt;
	*alt = new;
}

WARN_UNUSED_RESULT
static int
permute_cases(int invisible, struct ast_alt **alt, const struct txt *t)
{
	size_t i, j;
	unsigned long num_alphas, perm_count;
	unsigned long alpha_inds[CHAR_BIT * sizeof i - 1]; /* - 1 because we shift (1 << n) by this size */
	size_t n;
	char *p;

	assert(alt != NULL);
	assert(t != NULL);
	assert(t->p != NULL);
	
	p = (void *) t->p;
	n = t->n;
	
	num_alphas = 0;
	for (i = 0; i < n; i++) {
		if (!isalpha((unsigned char) p[i])) {
			continue;
		}

		if (num_alphas + 1 > sizeof alpha_inds / sizeof *alpha_inds) {
			fprintf(stderr, "Too many alpha characters in case-invensitive string "
				"\"%.*s\", max is %u\n",
				(int) t->n, t->p,
				(unsigned) (sizeof alpha_inds / sizeof *alpha_inds));
			return 0;
		}

		alpha_inds[num_alphas++] = i;
	}

	perm_count = (1UL << num_alphas); /* this limits us to sizeof perm_count */
	for (i = 0; i < perm_count; i++) {
		for (j = 0; j < num_alphas; j++) {
			p[alpha_inds[j]] = ((i >> j) & 1UL)
				? tolower((unsigned char) p[alpha_inds[j]])
				: toupper((unsigned char) p[alpha_inds[j]]);
		}
		
		add_alt(invisible, alt, t);
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
rewrite_ci(struct ast_term *term)
{
	struct txt tmp;
	size_t i;

	assert(term->type == TYPE_CI_LITERAL);

	/* case is normalised during AST creation */
	for (i = 0; i < term->u.literal.n; i++) {
		if (!isalpha((unsigned char) term->u.literal.p[i])) {
			continue;
		}

		assert(islower((unsigned char) term->u.literal.p[i]));
	}

	assert(term->u.literal.p != NULL);
	tmp = term->u.literal;

	/* we repurpose the existing node, which breaks abstraction for freeing */
	term->type = TYPE_GROUP;
	term->u.group = NULL;

	/* invisibility of new alts is inherited from term->invisible itself */
	int success = permute_cases(term->invisible, &term->u.group, &tmp);

	free((void *) tmp.p);
	return success;
}

WARN_UNUSED_RESULT
static int
walk_term(struct ast_term *term)
{
	assert(term != NULL);

	switch (term->type) {
	case TYPE_EMPTY:
	case TYPE_CS_LITERAL:
	case TYPE_TOKEN:
	case TYPE_PROSE:
		break;

	case TYPE_GROUP:
		walk_alts(term->u.group);
		return 1;

	case TYPE_RULE:
		/* (struct ast_term).u.rule is just for the name; don't recurr into it */
		return 1;

	case TYPE_CI_LITERAL:
		return rewrite_ci(term);
	}
}

WARN_UNUSED_RESULT
static int
walk_alts(struct ast_alt *alts)
{
	struct ast_alt *alt;
	struct ast_term *term;

	for (alt = alts; alt != NULL; alt = alt->next) {
		for (term = alt->terms; term != NULL; term = term->next) {
			if (!walk_term(term))
				return 0;
		}
	}

	return 1;
}

WARN_UNUSED_RESULT
int
rewrite_ci_literals(struct ast_rule *grammar)
{
	struct ast_rule *rule;

	for (rule = grammar; rule != NULL; rule = rule->next) {
		if (!walk_alts(rule->alts))
			return 0;
	}
	return 1;
}

