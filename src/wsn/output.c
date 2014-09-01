/* $Id$ */

/*
 * Wirth Syntax Notation Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"

static void output_term(struct ast_term *term);

static void
output_group_alt(struct ast_alt *alt)
{
	struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);
	}
}

static void
output_group(struct ast_group *group)
{
	char s, e;
	struct ast_alt *alt;

	assert(group->kleene != KLEENE_CROSS);

	switch (group->kleene) {
	case KLEENE_STAR:     s = '{'; e = '}'; break;
	case KLEENE_GROUP:    s = '('; e = ')'; break;
	case KLEENE_OPTIONAL: s = '['; e = ']'; break;
	}

	printf(" %c", s);

	for (alt = group->alts; alt != NULL; alt = alt->next) {
		output_group_alt(alt);

		if (alt->next != NULL) {
			printf(" |");
		}
	}

	printf(" %c", e);
}

static void
output_term(struct ast_term *term)
{
	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_PRODUCTION:
		printf(" %s", term->u.name);
		break;

	case TYPE_TERMINAL: {
			const char *p;

			fputs(" \"", stdout);
			for (p = term->u.literal; *p != '\0'; p++) {
				if (*p == '\"') {
					fputs("\"\"", stdout);
				} else {
					putc(*p, stdout);
				}
			}
			putc('\"', stdout);
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
	}
}

static void
output_production(struct ast_production *production)
{
	struct ast_alt *alt;

	alt = production->alts;
	printf("%s =", production->name);
	output_alt(alt);

	for (alt = alt->next; alt != NULL; alt = alt->next) {
		printf("\n\t|");
		output_alt(alt);
	}

	printf(" .\n\n");
}

void
wsn_output(struct ast_production *grammar)
{
	struct ast_production *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_production(p);
	}
}

