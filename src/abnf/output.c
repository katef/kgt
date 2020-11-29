/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../bitmap.h"
#include "../rrd/node.h"
#include "../compiler_specific.h"

#include "io.h"

WARN_UNUSED_RESULT
static int output_term(const struct ast_term *term);

static void
output_byte(char c)
{
	printf("%%x%02X", (unsigned char) c);
}

static void
output_range(char lo, char hi)
{
	printf("%%x%02X-%02X", (unsigned char) lo, (unsigned char) hi);
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

WARN_UNUSED_RESULT
static int
output_string(char prefix, const struct txt *t)
{
	size_t i;

	assert(t != NULL);

	if (t->n == 1 && needesc(*t->p)) {
		output_byte(*t->p);
		return 1;
	}

	if (txthas(t, needesc)) {
		fprintf(stderr, "unsupported: escaping special characters within a literal\n");
		return 0;
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
	return 1;
}

static int
char_terminal(const struct ast_term *term, unsigned char *c)
{
	assert(c != NULL);

	/* one terminal only */
	if (term == NULL || term->next != NULL) {
		return 0;
	}

	/* we collate ranges for case-sensitive strings only */
	if (term->type != TYPE_CS_LITERAL) {
		return 0;
	}

	if (term->u.literal.n != 1) {
		return 0;
	}

	*c = (unsigned) term->u.literal.p[0];

	return 1;
}

static void
collate_ranges(struct bm *bm, const struct ast_alt *alts)
{
	const struct ast_alt *p;

	assert(bm != NULL);

	bm_clear(bm);

	for (p = alts; p != NULL; p = p->next) {
		unsigned char c;

		if (!char_terminal(p->terms, &c)) {
			continue;
		}

		bm_set(bm, c);
	}
}

WARN_UNUSED_RESULT
static int
output_terms(const struct ast_term *terms)
{
	const struct ast_term *term;

	for (term = terms; term != NULL; term = term->next) {
		if (!output_term(term))
			return 0;

		if (term->next) {
			putc(' ', stdout);
		}
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_alts(const struct ast_alt *alts)
{
	const struct ast_alt *p;
	struct bm bm;
	int hi, lo;
	int first;

	collate_ranges(&bm, alts);

	hi = -1;

	p = alts;

	first = 1;

	while (p != NULL) {
		unsigned char c;

		if (!char_terminal(p->terms, &c)) {
			if (!first) {
				printf(" / ");
			} else {
				first = 0;
			}

			if (!output_terms(p->terms))
				return 0;
			p = p->next;
			continue;
		}

		if (!bm_get(&bm, c)) {
			/* already output */
			p = p->next;
			continue;
		}

		if (!first) {
			printf(" / ");
		} else {
			first = 0;
		}

		/* start of range */
		lo = bm_next(&bm, hi, 1);
		if (lo > UCHAR_MAX) {
			/* end of list */
			break;
		}

		/* end of range */
		hi = bm_next(&bm, lo, 0);

		/*
		 * Character classes aren't relevant for ABNF
		 * since we can only output hex escapes.
		 */

		assert(hi > lo);

		switch (hi - lo) {
			int j;

		case 1:
		case 2:
		case 3:
			{
				struct txt t;
				char a[1];
				a[0] = (unsigned char) lo;
				t.p = a;
				t.n = sizeof a / sizeof *a;
				if (!output_string('s', &t))
					return 0;
			}
			bm_unset(&bm, lo);

			hi = lo;
			break;

		default:
			output_range(lo, hi - 1);

			for (j = lo; j <= hi - 1; j++) {
				bm_unset(&bm, j);
			}

			break;
		}
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_group(const struct ast_alt *group)
{
	if (group->next != NULL) {
		printf("(");
	}

	if (!output_alts(group))
		return 0;

	if (group->next != NULL) {
		printf(")");
	}
	return 1;
}

static void
output_repetition(unsigned int min, unsigned int max)
{
	if (min == 0 && max == 1) {
		assert(!"unreached");
	}

	if (min == 1 && max == 1) {
		/* no operator */
		return;
	}

	if (min != 0 && min == max) {
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

	assert(!"unreached");
}

WARN_UNUSED_RESULT
static int
output_term(const struct ast_term *term)
{
	int a;

	assert(term != NULL);

	a = atomic(term);

	if (term->min == 0 && term->max == 1) {
		printf("[ ");
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
		if (!output_string('i', &term->u.literal))
			return 0;
		break;

	case TYPE_CS_LITERAL:
		if (!output_string('s', &term->u.literal))
			return 0;
		break;

	case TYPE_TOKEN:
		printf("%s", term->u.token);
		break;

	case TYPE_PROSE:
		/* TODO: escaping to somehow avoid > */
		printf("< %s >", term->u.prose);
		break;

	case TYPE_GROUP:
		if (!output_group(term->u.group))
			return 0;
		break;
	}

	if (term->min == 0 && term->max == 1) {
		printf(" ]");
	} else if (!a) {
		printf(" )");
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_rule(const struct ast_rule *rule)
{
	printf("%s = ", rule->name);

	if (!output_alts(rule->alts))
		return 0;

	printf("\n");
	printf("\n");
	return 1;
}

WARN_UNUSED_RESULT
int
abnf_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		if (!output_rule(p))
			return 0;
	}
	return 1;
}

