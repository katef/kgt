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
#include "../main.h"

static void output_alts(struct ast_alt *alts);

static void
output_group(struct ast_group *group)
{
	const char *s, *e;

	assert(group->kleene != KLEENE_CROSS);

	switch (group->kleene) {
	case KLEENE_STAR:     s = "ZeroOrMore("; e = ")"; break;
	case KLEENE_CROSS:    s = "OneOrMore("; e = ")";  break;
	case KLEENE_GROUP:    s = "Sequence("; e = ")";   break;
	case KLEENE_OPTIONAL: s = "Optional("; e = ")";   break;
	}

	printf("%s", s);

	output_alts(group->alts);

	printf("%s", e);
}

static void
output_term(struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:
		fputs("Skip()", stdout);
		break;

	case TYPE_PRODUCTION:
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
		output_group(term->u.group);
		break;
	}
}

static void
output_alt(struct ast_alt *alt)
{
	struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);

		if (term->next != NULL) {
			printf(", ");
		}
	}
}

static void
output_alts(struct ast_alt *alts)
{
	struct ast_alt *alt;

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
output_production(struct ast_production *production)
{
	printf("add('%s', Diagram(\n\t", production->name);

	output_alts(production->alts);

	printf("));\n\n");
}

void
trd_output(struct ast_production *grammar)
{
	struct ast_production *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_production(p);
	}
}

