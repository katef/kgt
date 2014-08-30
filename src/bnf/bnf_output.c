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

static void output_term(struct ast_term *term) {
	assert(term->type != TYPE_GROUP);
	/* TODO: semantic checks ought to find if we can output to this language; groups cannot */

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_PRODUCTION:
		printf(" <%s>", term->u.name);
		break;

	case TYPE_TERMINAL: {
			char c;

			c = strchr(term->u.literal, '\"') ? '\'' : '\"';
			printf(" %c%s%c", c, term->u.literal, c);
		}
		break;
	}
}

static void output_alt(struct ast_alt *alt) {
	struct ast_term *term;

	for (term = alt->terms; term; term = term->next) {
		output_term(term);
	}

	printf("\n");
}

static void output_production(struct ast_production *production) {
	struct ast_alt *alt;

	alt = production->alts;
	printf("<%s> ::=", production->name);
	output_alt(alt);

	for (alt = alt->next; alt; alt = alt->next) {
		printf("\t|");
		output_alt(alt);
	}

	printf("\n");
}

void bnf_output(struct ast_production *grammar) {
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		output_production(p);
	}
}

