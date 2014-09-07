/* $Id$ */

/*
 * Graphviz Dot Diagram Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../ast.h"

#include "io.h"

static void output_alt(const struct ast_rule *grammar, const struct ast_alt *alt);

static void
escputc(int c, FILE *f)
{
	size_t i;

	struct {
		int in;
		const char *out;
	} esc[] = {
		{ '&',  "&amp;"  },
		{ '\"', "&quot;" },
		{ '<',  "&#x3C;" },
		{ '>',  "&#x3E;" },

		{ '\\', "\\\\"   },
		{ '\f', "\\f"    },
		{ '\n', "\\n"    },
		{ '\r', "\\r"    },
		{ '\t', "\\t"    },
		{ '\v', "\\v"    },

		{ '|',  "\\|"    },
		{ '{',  "\\{"    },
		{ '}',  "\\}"    }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof esc / sizeof *esc; i++) {
		if (esc[i].in == c) {
			fputs(esc[i].out, f);
			return;
		}
	}

	if (!isprint(c)) {
		fprintf(f, "\\x%x", (unsigned char) c);
		return;
	}

	putc(c, f);
}

static void
escputs(const char *s, FILE *f)
{
	const char *p;

	for (p = s; *p != '\0'; p++) {
		escputc(*p, f);
	}
}

static void
output_group(const struct ast_rule *grammar,
	const struct ast_term *term, const struct ast_alt *group)
{
	const struct ast_alt *alt;

	for (alt = group; alt != NULL; alt = alt->next) {
		printf("\t\"t%p\" -> \"a%p\";\n",
			(void *) term, (void *) alt);

		output_alt(grammar, alt);
	}
}

static void
output_term(const struct ast_rule *grammar,
	const struct ast_alt *alt, const struct ast_term *term)
{
	assert(term->max >= term->min || !term->max);

	printf("\t\"a%p\" -> \"t%p\";\n",
		(void *) alt, (void *) term);

	printf("\t\"t%p\" [ shape = record, label = \"",
		(void *) term);

	if (term->min == 1 && term->max == 1) {
		/* nothing */
	} else if (!term->max) {
		printf("\\{%u,""\\}&times;|", term->min);
	} else if (term->min == term->max) {
		printf("%u&times;|", term->min);
	} else {
		printf("\\{%u,%u\\}&times;|", term->min, term->max);
	}

	switch (term->type) {
	case TYPE_EMPTY:
		fputs("&#x3B5;", stdout);
		break;

	case TYPE_RULE:
		escputs(term->u.name, stdout);
		break;

	case TYPE_TERMINAL:
		escputs(term->u.literal, stdout);
		break;

	case TYPE_GROUP:
		printf("()");
		break;
	}

	printf("\" ];\n");

	switch (term->type) {
	case TYPE_EMPTY:
		break;

	case TYPE_RULE:
		/* XXX: the AST ought to have a link to the ast_rule here */
/* XXX: cross-links to rules are confusing
		if (term->ast_find_rule(grammar, term->u.name) != NULL) {
			printf("\t\"t%p\" -> \"p%p\" [ dir = forward, color = blue, weight = 0 ];\n",
				(void *) term, ast_find_rule(grammar, term->u.name));
		}
*/
		break;

	case TYPE_TERMINAL:
		printf("\t\"t%p\" [ style = filled ];\n",
			(void *) term);
		break;

	case TYPE_GROUP:
		output_group(grammar, term, term->u.group);
		break;
	}
}

static void
output_alt(const struct ast_rule *grammar,
	const struct ast_alt *alt)
{
	const struct ast_term *term;

	printf("\t\"a%p\" [ label = \"|\" ];\n",
		(void *) alt);

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(grammar, alt, term);
	}
}

static void
output_alts(const struct ast_rule *grammar,
	const struct ast_rule *rule, const struct ast_alt *alts)
{
	const struct ast_alt *alt;

	for (alt = alts; alt != NULL; alt = alt->next) {
		printf("\t\"p%p\" -> \"a%p\";\n",
			(void *) rule, (void *) alt);

		output_alt(grammar, alt);
	}
}

static void
output_rule(const struct ast_rule *grammar,
	const struct ast_rule *rule)
{
	printf("\t\"p%p\" [ shape = record, label = \"=|%s\" ];\n",
		(void *) rule, rule->name);

	output_alts(grammar, rule, rule->alts);
}

void
dot_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("digraph G {\n");
	printf("\tnode [ shape = box, style = rounded ];\n");
	printf("\tedge [ dir = none ];\n");

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(grammar, p);
	}

	printf("};\n");
}

