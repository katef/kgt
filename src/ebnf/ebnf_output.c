/* $Id$ */

/*
 * Extended Backus-Naur Form Output
 * As defined by ISO/IEC 14977:1996(E)
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"

static void output_term(struct ast_term *term);

static void output_group_alt(struct ast_alt *alt) {
	struct ast_term *term;

	for (term = alt->terms; term; term = term->next) {
		output_term(term);
	}
}

static void output_group(struct ast_group *group) {
	char s, e;
	struct ast_alt *alt;

	assert(group->kleene != KLEENE_CROSS);

	switch (group->kleene) {
	case KLEENE_STAR:
		s = '{'; e = '}';
		break;

	case KLEENE_GROUP:
		s = '('; e = ')';
		break;

	case KLEENE_OPTIONAL:
		s = '['; e = ']';
		break;
	}

	printf(" %c", s);

	output_group_alt(group->alts);
	for (alt = group->alts->next; alt; alt = alt->next) {
		printf(" |");
		output_group_alt(alt);
	}

	printf(" %c", e);
}

static void output_term(struct ast_term *term) {
	if (term->repeat > 1) {
		printf(" %u *", term->repeat);
	}

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

static void output_alt(struct ast_alt *alt) {
	struct ast_term *term;

	for (term = alt->terms; term; term = term->next) {
		output_term(term);

		if (term->next) {
			putc(',', stdout);
		}
	}
}

static void output_production(struct ast_production *production) {
	struct ast_alt *alt;

	alt = production->alts;
	printf("%s =", production->name);
	output_alt(alt);

	for (alt = alt->next; alt; alt = alt->next) {
		printf("\n\t|");
		output_alt(alt);
	}

	printf(";\n\n");
}

void ebnf_output(struct ast_production *grammar) {
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		output_production(p);
	}
}

