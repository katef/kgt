/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

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

#include "../txt.h"
#include "../ast.h"

#include "io.h"

static void output_alt(const struct ast_rule *grammar, const struct ast_alt *alt);

static int
escputc(int c, FILE *f)
{
	size_t i;

	const struct {
		int c;
		const char *s;
	} a[] = {
		{ '&',  "&amp;"  },
		{ '\"', "&quot;" },
		{ '<',  "&#x3C;" },
		{ '>',  "&#x3E;" }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (a[i].c == c) {
			return fputs(a[i].s, f);
		}
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "&#x%X;", (unsigned char) c);
	}

	return putc(c, f);
}

static int
escputs(const char *s, FILE *f)
{
	const char *p;
	int r;

	for (p = s; *p != '\0'; p++) {
		r = escputc(*p, f);
		if (r < 0) {
			return -1;
		}
	}

	return 0;
}

static int
escputt(const struct txt *t, FILE *f)
{
	size_t i;
	int r;

	assert(t != NULL);
	assert(t->p != NULL);

	for (i = 0; i < t->n; i++) {
		r = escputc(t->p[i], f);
		if (r < 0) {
			return -1;
		}
	}

	return 0;
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

	printf("\t\"a%p\" -> \"t%p\"",
		(void *) alt, (void *) term);
	if (term->invisible) {
		printf(" [ color = blue, style = dashed ]");
	}
	printf(";\n");

	printf("\t\"t%p\" [ shape = record, ",
		(void *) term);

	if (term->invisible) {
		printf("color = blue, fontcolor = blue, style = \"rounded,dashed\", ");
	}

	printf("label = \"");

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
		escputs(term->u.rule->name, stdout);
		break;

	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
		fputs("&quot;", stdout);
		escputt(&term->u.literal, stdout);
		fputs("&quot;", stdout);
		if (term->type == TYPE_CI_LITERAL) {
			fputs("/i", stdout);
		}
		break;

	case TYPE_TOKEN:
		escputs(term->u.token, stdout);
		break;

	case TYPE_PROSE:
		fputs("?", stdout);
		escputs(term->u.prose, stdout);
		fputs("?", stdout);
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
/* XXX: cross-links to rules are confusing
		printf("\t\"t%p\" -> \"p%p\" [ dir = forward, color = blue, weight = 0 ];\n",
			(void *) term, term->u.rule);
*/
		break;

	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
	case TYPE_TOKEN:
	case TYPE_PROSE:
		printf("\t\"t%p\" [ style = filled",
			(void *) term);

		if (term->invisible) {
			printf(", color = blue, fillcolor = aliceblue, style = \"dashed,filled\" ");
		}

		printf("];\n");
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

	printf("\t\"a%p\" [ label = \"|\"",
		(void *) alt);

if (alt->invisible) {
	printf(", color = blue ");
}

	printf("];\n");

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

WARN_UNUSED_RESULT
int
dot_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("digraph G {\n");
	printf("\tnode [ shape = box, style = rounded ];\n");
	printf("\tedge [ dir = none ];\n");

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(grammar, p);
	}

	printf("}\n");
	return 1;
}

