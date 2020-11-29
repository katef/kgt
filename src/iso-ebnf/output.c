/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Extended Backus-Naur Form Output
 * As defined by ISO/IEC 14977:1996(E)
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../txt.h"
#include "../ast.h"
#include "../rrd/node.h"
#include "../compiler_specific.h"

#include "io.h"

WARN_UNUSED_RESULT
static int output_term(const struct ast_term *term);

WARN_UNUSED_RESULT
static int
output_group_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		if (!output_term(term))
			return 0;
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_group(const struct ast_alt *group)
{
	const struct ast_alt *alt;

	for (alt = group; alt != NULL; alt = alt->next) {
		if (!output_group_alt(alt))
			return 0;

		if (alt->next != NULL) {
			printf(" |");
		}
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
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
		{ 1, 1, " (", " ) " },
		{ 1, 1, "",    ""   },
		{ 0, 1, " [", " ] " },
		{ 0, 0, " {", " } " }
	};

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

	assert(s != NULL && e != NULL);

	/* EBNF cannot express minimum term repetition; TODO: semantic checks for this */
	assert(term->min <= 1);

	printf("%s", s);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_RULE:
		printf(" %s", term->u.rule->name);
		break;

	case TYPE_CI_LITERAL:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case TYPE_CS_LITERAL: {
			size_t i;

			fputs(" \"", stdout);
			for (i = 0; i < term->u.literal.n; i++) {
				if (term->u.literal.p[i] == '\"') {
					fputs("\"\"", stdout);
				} else {
					putc(term->u.literal.p[i], stdout);
				}
			}
			putc('\"', stdout);
		}
		break;

	case TYPE_TOKEN:
		printf(" %s", term->u.token);
		break;

	case TYPE_PROSE:
		/* TODO: escaping to somehow avoid ? */
		printf(" ? %s ?", term->u.prose);
		break;

	case TYPE_GROUP:
		if (!output_group(term->u.group))
			return 0;
		break;
	}

	printf("%s", e);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		if (!output_term(term))
			return 0;

		if (term->next) {
			putc(',', stdout);
		}
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("%s =", rule->name);
	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		if (!output_alt(alt))
			return 0;

		if (alt->next != NULL) {
			printf("\n\t|");
		}
	}

	printf("\n");
	printf("\t;\n");
	printf("\n");
	return 1;
}

WARN_UNUSED_RESULT
int
iso_ebnf_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		if (!output_rule(p))
			return 0;
	}
	return 1;
}

