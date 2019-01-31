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
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../ast.h"

#include "io.h"

static void output_term(const struct ast_term *term);

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

static void
output_group_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	for (term = alt->terms; term != NULL; term = term->next) {
		output_term(term);
	}
}

static void
output_group(const struct ast_alt *group)
{
	const struct ast_alt *alt;

	for (alt = group; alt != NULL; alt = alt->next) {
		output_group_alt(alt);

		if (alt->next != NULL) {
			printf(" |");
		}
	}
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
			const char *p;

			putc(' ', stdout);

			/* XXX: the tokenization here is wrong; this should be a single token */

			for (p = term->u.literal; *p != '\0'; p++) {
				char uc, lc;

				uc = toupper((unsigned char) *p);
				lc = tolower((unsigned char) *p);

				if (uc == lc) {
					putc('[', stdout);
					(void) blab_escputc(stdout, *p);
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
			const char *p;

			fputs(" \"", stdout);

			for (p = term->u.literal; *p != '\0'; p++) {
				(void) blab_escputc(stdout, *p);
			}

			putc('\"', stdout);
		}
		break;

	case TYPE_TOKEN:
		printf(" %s", term->u.token);
		break;

	case TYPE_GROUP:
		output_group(term->u.group);
		break;
	}

	if (!a) {
		printf(" )");
	}

	output_repetition(term->min, term->max);
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

	printf("%s =", rule->name);
	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		output_alt(alt);

		if (alt->next != NULL) {
			printf("\n\t|");
		}
	}

	printf("\n");
	printf("\n");
}

void
blab_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}
}

