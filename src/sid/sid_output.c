/* $Id$ */

/*
 * SID Output.
 *
 * TODO: fprintf(fout), instead opf stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../ast.h"
#include "../main.h"

static void output_section(const char *section) {
	printf("\n%%%s%%\n\n", section);
}

static void output_term(struct ast_term *term) {
	assert(term->type != TYPE_GROUP);
	/* TODO: can output groups as [ ... ] local rules with a stub to call them X times? */

	switch (term->type) {
	case TYPE_EMPTY:
		fputs("$$; ", stdout);
		break;

	case TYPE_PRODUCTION:
		printf("%s; ", term->u.name);
		break;

	case TYPE_TERMINAL: {
			char c;

			c = strchr(term->u.literal, '\"') ? '\'' : '\"';
			printf("%c%s%c; ", c, term->u.literal, c);
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
	printf("\t%s = {\n\t\t", production->name);
	output_alt(alt);

	for (alt = alt->next; alt; alt = alt->next) {
		printf("\t||\n\t\t");
		output_alt(alt);
	}

	printf("\t};\n\n");
}

static struct ast_production *is_defined(struct ast_production *grammar, const char *name) {
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		if (strcmp(p->name, name) == 0) {
			return p;
		}
	}

	return NULL;
}

static int is_equal(struct ast_term *a, struct ast_term *b) {
	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
	case TYPE_PRODUCTION:
		return strcmp(a->u.name, b->u.name) == 0;

	case TYPE_TERMINAL:
		return strcmp(a->u.literal, b->u.literal) == 0;
	}
}

static void output_terminals(struct ast_production *grammar) {
	struct ast_production *p;
	struct ast_term *found = NULL;

	/* List terminals */
	for (p = grammar; p; p = p->next) {
		struct ast_alt *alt;

		for (alt = p->alts; alt; alt = alt->next) {
			struct ast_term *term;
			struct ast_term *t;

			for (term = alt->terms; term; term = term->next) {
				const char *name;

				if (term->type == TYPE_PRODUCTION && is_defined(grammar, term->u.name)) {
					continue;
				}

				for (t = found; t; t = t->next) {
					if (is_equal(t, term)) {
						break;
					}
				}

				if (t != NULL) {
					continue;
				}

				t = xmalloc(sizeof *t);
				t->u.name = term->u.name;
				t->type = term->type;
				t->next = found;
				found = t;
			}
		}
	}

	/* Output list */
	{
		struct ast_term *next;
		struct ast_term *t;

		for (t = found; t; t = next) {
			next = t->next;
			printf("\t");
			output_term(t);
			printf("\n");
			free(t);
		}
	}
}

void sid_output(struct ast_production *grammar) {
	struct ast_production *p;

	output_section("types");

	output_section("terminals");

	output_terminals(grammar);

	output_section("productions");

	/* TODO list production declartations */

	for (p = grammar; p; p = p->next) {
		output_production(p);
	}

	output_section("entry");

	printf("\t%s;\n\n", grammar->name);
}

