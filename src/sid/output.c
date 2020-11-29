/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * SID Output.
 *
 * TODO: fprintf(fout), instead of stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../xalloc.h"
#include "../compiler_specific.h"

#include "io.h"

WARN_UNUSED_RESULT
static int output_alt(const struct ast_alt *);

static void
output_section(const char *section)
{
	printf("\n%%%s%%\n\n", section);
}

static void
output_literal(const struct txt *t)
{
	char c;

	assert(t != NULL);
	assert(t->p != NULL);

	c = memchr(t->p, '\"', t->n) ? '\'' : '\"';
	printf("%c%.*s%c; ", c, (int) t->n, t->p, c);
}

WARN_UNUSED_RESULT
static int
output_basic(const struct ast_term *term)
{
	assert(term != NULL);
	assert(!term->invisible);

	switch (term->type) {
	case TYPE_EMPTY:
		fputs("$$; ", stdout);
		break;

	case TYPE_RULE:
		printf("%s; ", term->u.rule->name);
		break;

	case TYPE_CI_LITERAL:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case TYPE_CS_LITERAL:
		output_literal(&term->u.literal);
		break;

	case TYPE_TOKEN:
		printf("%s; ", term->u.token);
		break;

	case TYPE_PROSE:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case TYPE_GROUP:
		fputs("{ ", stdout);
		if (!output_alt(term->u.group))
			return 0;
		fputs("}; ", stdout);
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term(const struct ast_term *term)
{
	assert(term != NULL);
	assert(!term->invisible);

	/* SID cannot express term repetition; TODO: semantic checks for this */
	/* TODO: can output repetition as [ ... ] local rules with a stub to call them X times? */
	assert(term->min <= 1);
	assert(term->max <= 1);

	if (term->min == 0) {
		fputs("{ $$; || ", stdout);
	}

	if (!output_basic(term))
		return 0;

	if (term->min == 0) {
		fputs("}; ", stdout);
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	assert(alt != NULL);
	assert(!alt->invisible);

	for (term = alt->terms; term != NULL; term = term->next) {
		if (!output_term(term))
			return 0;
	}
	return 1;
}

WARN_UNUSED_RESULT
static int
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("\t%s = {\n\t\t", rule->name);

	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		if (!output_alt(alt))
			return 0;

		printf("\n");

		if (alt->next != NULL) {
			printf("\t||\n\t\t");
		}
	}

	printf("\t};\n\n");
	return 1;
}

static int
is_equal(const struct ast_term *a, const struct ast_term *b)
{
	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
	case TYPE_EMPTY:      return 1;
	case TYPE_RULE:       return 0 == strcmp(a->u.rule->name   , b->u.rule->name);
	case TYPE_CI_LITERAL: return 0 == txtcasecmp(&a->u.literal , &b->u.literal);
	case TYPE_CS_LITERAL: return 0 == txtcmp(&a->u.literal     , &b->u.literal);
	case TYPE_TOKEN:      return 0 == strcmp(a->u.token        , b->u.token);
	case TYPE_PROSE:      return 0 == strcmp(a->u.prose        , b->u.prose);

	case TYPE_GROUP:
	/* unimplemented */
	return 0;
	}

	assert(!"unreached");
}

WARN_UNUSED_RESULT
static int
output_terminals(const struct ast_rule *grammar)
{
	const struct ast_rule *p;
	struct ast_term *found = NULL;

	/* List terminals */
	for (p = grammar; p != NULL; p = p->next) {
		struct ast_alt *alt;

		for (alt = p->alts; alt != NULL; alt = alt->next) {
			const struct ast_term *term;
			struct ast_term *t;

			assert(alt!= NULL);
			assert(!alt->invisible);

			for (term = alt->terms; term != NULL; term = term->next) {
				assert(term!= NULL);
				assert(!term->invisible);

				switch (term->type) {
				case TYPE_EMPTY:
				case TYPE_GROUP:
					continue;

				case TYPE_RULE:
				case TYPE_TOKEN:
				case TYPE_PROSE:
					continue;

				case TYPE_CI_LITERAL:
				case TYPE_CS_LITERAL:
					break;
				}

				for (t = found; t != NULL; t = t->next) {
					if (is_equal(t, term)) {
						break;
					}
				}

				if (t != NULL) {
					continue;
				}

				t = xmalloc(sizeof *t);
				t->u.literal = term->u.literal;
				t->type = term->type;
				t->invisible = 0;
				t->next = found;
				found = t;
			}
		}
	}

	/* Output list */
	{
		struct ast_term *next;
		struct ast_term *t;

		for (t = found; t != NULL; t = next) {
			next = t->next;
			printf("\t");
			if (!output_basic(t))
				return 0;
			printf("\n");
			free(t);
		}
	}
	return 1;
}

WARN_UNUSED_RESULT
int
sid_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	output_section("types");

	output_section("terminals");

	if (!output_terminals(grammar))
		return 0;

	output_section("rules");

	/* TODO list rule declartations */

	for (p = grammar; p != NULL; p = p->next) {
		if (!output_rule(p))
			return 0;
	}

	output_section("entry");

	printf("\t%s;\n\n", grammar->name);
	return 1;
}
