/* $Id$ */

/*
 * Backus-Naur Form Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"

#include "io.h"

static void
output_term(const struct ast_term *term)
{
	assert(term->type != TYPE_GROUP);
	/* TODO: semantic checks ought to find if we can output to this language; groups cannot */

	/* BNF cannot express term repetition; TODO: semantic checks for this */
	assert(term->min == 1);
	assert(term->max == 1);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_RULE:
		printf(" <%s>", term->u.rule->name);
		break;

	case TYPE_LITERAL: {
			char c;

			c = strchr(term->u.literal, '\"') ? '\'' : '\"';
			printf(" %c%s%c", c, term->u.literal, c);
		}
		break;
	}
}

static void
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);
	}

	printf("\n");
}

static void
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("<%s> ::=", rule->name);

	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		output_alt(alt);

		if (alt->next != NULL) {
			printf("\t|");
		}
	}

	printf("\n");
}

void
bnf_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}
}

