/*
 * Copyright 2018 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Blab grammar-based data generator
 * https://github.com/aoh/blab
 *
 * This is an EBNF-like dialect with regexp-style operators.
 * It isn't formally specified; output here is an attempt to match
 * the current syntax at the time of writing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../compiler_specific.h"

#include "io.h"

WARN_UNUSED_RESULT
static int output_term(const struct ast_term *term);

int
blab_escputc(FILE *f, char c)
{
	assert(f != NULL);

	switch (c) {
	case '\"': return fputs("\\\"", f);
	case '\\': return fputs("\\\\", f);
	case '\n': return fputs("\\n",  f);
	case '\r': return fputs("\\r",  f);
	case '\t': return fputs("\\t",  f);
	case '\'': return fputs("\\\'", f);

	default:
		break;
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "\\x%02x", (unsigned char) c);
	}

	return fprintf(f, "%c", c);
}

WARN_UNUSED_RESULT
static int
output_group_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		if (!output_term(term))
			return 0;
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_group(const struct ast_alt *group)
{
	const struct ast_alt *alt;

	for (alt = group; alt != NULL; alt = alt->next) {
		if (!output_group_alt(alt))
			return 0;

		if (alt->next != NULL) {
			printf(" |");
		}
	}
	return 1;
}

static void
output_repetition(unsigned int min, unsigned int max)
{
	if (min == 0 && max == 0) {
		printf("*");
	} else if (min == 0 && max == 1) {
		printf("?");
	} else if (min == 1 && max == 0) {
		printf("+");
	} else if (min == 1 && max == 1) {
		/* no operator */
	} else if (min == max) {
		printf("{%u}", min);
	} else if (max == 0) {
		printf("{%u,}", min);
	} else {
		printf("{%u,%u}", min, max);
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
	assert(!term->invisible);

	a = atomic(term);

	if (!a) {
		printf(" (");
	}

	switch (term->type) {
	case TYPE_EMPTY:
		fputs(" \"\"", stdout);
		break;

	case TYPE_RULE:
		printf(" %s", term->u.rule->name);
		break;

	case TYPE_CI_LITERAL: {
			size_t i;

			putc(' ', stdout);

			/* XXX: the tokenization here is wrong; this should be a single token */

			for (i = 0; i < term->u.literal.n; i++) {
				char uc, lc;

				uc = toupper((unsigned char) term->u.literal.p[i]);
				lc = tolower((unsigned char) term->u.literal.p[i]);

				if (uc == lc) {
					putc('[', stdout);
					(void) blab_escputc(stdout, term->u.literal.p[i]);
					putc(']', stdout);
					continue;
				}

				if (uc != lc) {
					putc('[', stdout);
					(void) blab_escputc(stdout, lc);
					(void) blab_escputc(stdout, uc);
					putc(']', stdout);
				}
			}
		}
		break;

	case TYPE_CS_LITERAL: {
			size_t i;

			fputs(" \"", stdout);

			for (i = 0; i < term->u.literal.n; i++) {
				(void) blab_escputc(stdout, term->u.literal.p[i]);
			}

			putc('\"', stdout);
		}
		break;

	case TYPE_TOKEN:
		printf(" %s", term->u.token);
		break;

	case TYPE_PROSE:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case TYPE_GROUP:
		if (!output_group(term->u.group))
			return 0;
		break;
	}

	if (!a) {
		printf(" )");
	}

	output_repetition(term->min, term->max);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	assert(!alt->invisible);

	for (term = alt->terms; term != NULL; term = term->next) {
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
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("%s =", rule->name);
	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		if (!output_alt(alt))
			return 0;

		if (alt->next != NULL) {
			printf("\n\t|");
		}
	}

	printf("\n");
	printf("\n");
	return 1;
}

WARN_UNUSED_RESULT
int
blab_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		if (!output_rule(p))
			return 0;
	}
	return 1;
}

