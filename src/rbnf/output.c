/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * RFC 5511 RBNF Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../txt.h"
#include "../ast.h"

#include "io.h"

static void output_term(const struct ast_term *term);

static void
output_group_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);
	}
}

static void
output_group(const struct ast_alt *group)
{
	const struct ast_alt *alt;

	for (alt = group; alt != NULL; alt = alt->next) {
		output_group_alt(alt);

		if (alt->next != NULL) {
			printf(" |");
		}
	}
}

static void
output_term(const struct ast_term *term)
{
	const char *s, *e;
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		const char *s;
		const char *e;
	} a[] = {
		{ 1, 1, " (", " ) "     },
		{ 1, 1, "",    ""       },
		{ 0, 1, " [", " ] "     },
		{ 0, 0, " [", " ... ] " }
	};

	assert(term != NULL);
	assert(!term->invisible);

	s = NULL;
	e = NULL;

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (i == 0 && term->type != TYPE_GROUP) {
			continue;
		}

		if (term->min == a[i].min && term->min == a[i].min) {
			s = a[i].s;
			e = a[i].e;
			break;
		}
	}

	/* RBNF cannot express min/max repetition; TODO: semantic checks for this */
	assert(s != NULL && e != NULL);

	/* TODO: RBNF cannot express literal <>::= etc. maybe output those as rule names */

	printf("%s", s);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_RULE:
		printf(" <%s>", term->u.rule->name);
		break;

	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
		fprintf(stderr, "unimplemented\n");
		exit(EXIT_FAILURE);

	case TYPE_TOKEN:
		printf(" <%s>", term->u.token);
		break;

	case TYPE_PROSE:
		fprintf(stderr, "unimplemented\n");
		exit(EXIT_FAILURE);

	case TYPE_GROUP:
		output_group(term->u.group);
		break;
	}

	printf("%s", e);
}

static void
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	assert(alt != NULL);
	assert(!alt->invisible);

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
rbnf_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}
}

