/* $Id$ */

/*
 * Tab Atkins Jr. Railroad Diagram Output.
 * See https://github.com/tabatkins/railroad-diagrams
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"

#include "io.h"

static void output_alts(const struct ast_alt *alts);

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
		{ 1, 1, "Sequence(",   ")" },
		{ 0, 1, "Optional(",   ")" },
		{ 0, 0, "ZeroOrMore(", ")" },
		{ 1, 0, "OneOrMore(",  ")" }
	};

	s = NULL;
	e = NULL;

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->min == a[i].min) {
			s = a[i].s;
			e = a[i].e;
			break;
		}
	}

	assert(s != NULL && e != NULL);

	/* the trd syntax cannot express minimum term repetition; TODO: semantic checks for this */
	assert(term->min <= 1);

	/* EBNF cannot express minimum term repetition; TODO: semantic checks for this */
	assert(term->min <= 1);
	assert(!(term->min == 1 || !term->max));

	printf(" %s", s);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs("Skip()", stdout);
		break;

	case TYPE_RULE:
		/* TODO: escape things */
		printf("NonTerminal('%s')", term->u.name);
		break;

	case TYPE_TERMINAL: {
			const char *p;

			/* TODO: escape things */
			printf("Terminal('");

			for (p = term->u.literal; *p != '\0'; p++) {
				if (*p == '\'') {
						fputs("\\'", stdout);
				} else {
						putc(*p, stdout);
				}
			}

			printf("')");
		}
		break;

	case TYPE_GROUP:
		output_alts(term->u.group);
		break;
	}

	if (term->max > 1) {
		printf(", Comment('%s%u')", "\xC3\x97", term->max);
	}

	printf("%s", e);
}

static void
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);

		if (term->next != NULL) {
			printf(", ");
		}
	}
}

static void
output_alts(const struct ast_alt *alts)
{
	const struct ast_alt *alt;

	if (alts->next == NULL) {
		output_alt(alts);
	} else {
		/* TODO: indent */
		printf("Choice(0,\n");

		for (alt = alts; alt != NULL; alt = alt->next) {
			printf("\t\t");

			output_alt(alt);

			if (alt->next != NULL) {
				printf(",\n");
			}
		}

		printf(")");
	}
}

static void
output_rule(const struct ast_rule *rule)
{
	printf("add('%s', Diagram(\n\t", rule->name);

	output_alts(rule->alts);

	printf("));\n\n");
}

void
trd_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}
}

