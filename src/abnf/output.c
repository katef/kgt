/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../rrd/node.h"

#include "io.h"

static void output_alt(const struct ast_alt *alt);

static void
output_group(const struct ast_alt *group)
{
	const struct ast_alt *alt;

	if (group->next != NULL) {
		printf("(");
	}

	for (alt = group; alt != NULL; alt = alt->next) {
		output_alt(alt);

		if (alt->next != NULL) {
			printf(" / ");
		}
	}

	if (group->next != NULL) {
		printf(")");
	}
}

static int
needesc(int c)
{
	if (!isprint((unsigned char) c)) {
		return 1;
	}

	switch (c) {
	case '"':
	case '\a':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return 1;

	default:
		return 0;
	}
}

static int
txthas(const struct txt *t, int (*f)(int c))
{
	size_t i;

	assert(t != NULL);

	for (i = 0; i < t->n; i++) {
		if (f(t->p[i])) {
			return 1;
		}
	}

	return 0;
}

static void
output_string(char prefix, const struct txt *t)
{
	size_t i;

	assert(t != NULL);

	if (t->n == 1 && needesc(*t->p)) {
		printf("%%x%02X", (unsigned char) *t->p);
		return;
	}

	if (txthas(t, needesc)) {
		fprintf(stderr, "unsupported: escaping special characters within a literal\n");
		exit(EXIT_FAILURE);
	}

	if (txthas(t, isalpha)) {
		printf("%%%c", prefix);
	}

	putc('\"', stdout);

	/* TODO: bail out on non-printable characters */

	for (i = 0; i < t->n; i++) {
		putc(t->p[i], stdout);
	}

	putc('\"', stdout);
}

static void
output_repetition(unsigned int min, unsigned int max)
{
	if (min == 0 && max == 0) {
		assert(!"unreached");
	}

	if (min == 0 && max == 1) {
		assert(!"unreached");
	}

	if (min == 1 && max == 1) {
		/* no operator */
		return;
	}

	if (min == max) {
		printf("%u", min);
		return;
	}

	if (min > 0) {
		printf("%u", min);
	}

	printf("*");

	if (max > 0) {
		printf("%u", max);
	}
}

static int
atomic(const struct ast_term *term)
{
	assert(term != NULL);

	if (term->min == 1 && term->max == 1) {
		return 1;
	}

	switch (term->type) {
	case TYPE_EMPTY:
	case TYPE_RULE:
	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
	case TYPE_TOKEN:
	case TYPE_PROSE:
		return 1;

	case TYPE_GROUP:
		return 0;
	}
}

static void
output_term(const struct ast_term *term)
{
	int a;

	assert(term != NULL);

	a = atomic(term);

	if (term->min == 0 && term->max == 1) {
		printf("[ ");
	} else if (term->min == 0 && term->max == 0) {
		printf("{ ");
	} else {
		output_repetition(term->min, term->max);

		if (!a) {
			printf("( ");
		}
	}

	switch (term->type) {
	case TYPE_EMPTY:
		fputs("\"\"", stdout);
		break;

	case TYPE_RULE:
		printf("%s", term->u.rule->name);
		break;

	case TYPE_CI_LITERAL:
		output_string('i', &term->u.literal);
		break;

	case TYPE_CS_LITERAL:
		output_string('s', &term->u.literal);
		break;

	case TYPE_TOKEN:
		printf("%s", term->u.token);
		break;

	case TYPE_PROSE:
		printf("< %s >", term->u.prose);
		exit(EXIT_FAILURE);

	case TYPE_GROUP:
		output_group(term->u.group);
		break;
	}

	if (term->min == 0 && term->max == 1) {
		printf(" ]");
	} else if (term->min == 0 && term->max == 0) {
		printf(" }");
	} else if (!a) {
		printf(" )");
	}
}

static void
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);

		if (term->next) {
			putc(' ', stdout);
		}
	}
}

static void
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("%s = ", rule->name);
	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		output_alt(alt);

		if (alt->next != NULL) {
			printf("\n\t/ ");
		}
	}

	printf("\n");
	printf("\n");
}

void
abnf_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}
}

