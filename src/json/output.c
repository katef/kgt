/*
 * Copyright 2021 John Scott
 *
 * See LICENCE for the full copyright terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"

#include "io.h"

int
escputc(char c)
{
    /*
     A simple way to deal with all escaped characters:
        '0020' . '10FFFF' - '"' - '\'
     is to convert them all to the valid '\u' hex hex hex hex format.
     */
    
    if ((unsigned char) c == '\"' || (unsigned char) c == '\\' || (unsigned char) c < ' ') {
        return printf("\\u00%02x", (unsigned char) c);
    }
    
    return printf("%c", c);
}

void output_string(const char *string) {
    fputs("\"", stdout);

    for(; *string ; string++) {
        escputc(*string);
    }

    fputs("\"", stdout);
}

void output_txt(const struct txt t) {
	size_t i;
	fputs("\"", stdout);
	for (i = 0; i < t.n; i++) {
        escputc(t.p[i]);
	}
	fputs("\"", stdout);
}


WARN_UNUSED_RESULT
static int
output_alts(const struct ast_alt *alts);


WARN_UNUSED_RESULT
static int
output_term_rule(const struct ast_rule *rule) {
	fputs(",", stdout);
	output_string("rule");
	fputs(":", stdout);
	output_string(rule->name);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term_token(const char *token) {
	fputs(",", stdout);
	output_string("token");
	fputs(":", stdout);
	output_string(token);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term_prose(const char *prose) {
	fputs(",", stdout);
	output_string("prose");
	fputs(":", stdout);
	output_string(prose);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term_group(const struct ast_alt *group) {
	fputs(",", stdout);
	output_string("group");
	fputs(":", stdout);
	if (!output_alts(group))
		return 0;
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term_literal(const struct txt literal) {
	size_t i;
	fputs(",", stdout);
	output_string("literal");
	fputs(":", stdout);
	output_txt(literal);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_term(const struct ast_term *term)
{
	fputs("{", stdout);
	output_string("$isa");
	fputs(":", stdout);
	switch (term->type) {
		case TYPE_EMPTY: output_string("empty"); break;
		case TYPE_RULE: output_string("rule"); break;
		case TYPE_CS_LITERAL: output_string("cs_literal"); break;
		case TYPE_CI_LITERAL: output_string("ci_literal"); break;
		case TYPE_TOKEN: output_string("token"); break;
		case TYPE_PROSE: output_string("prose"); break;
		case TYPE_GROUP: output_string("group"); break;
		default: fputs("null", stdout); break;
	}

	if (term->min != 1) {
		fputs(",", stdout);
		output_string("min");
		fputs(":", stdout);
		printf("%d", term->min);
	}

	if (term->max != 1) {
		fputs(",", stdout);
		output_string("max");
		fputs(":", stdout);
		printf("%d", term->max);
	}

	if (term->invisible != 0) {
		fputs(",", stdout);
		output_string("invisible");
		fputs(":true", stdout);
	}

	switch (term->type) {
		case TYPE_EMPTY:
			break;
		case TYPE_RULE:
			if (!output_term_rule(term->u.rule))
				return 0;
			break;
		case TYPE_CS_LITERAL:
		case TYPE_CI_LITERAL:
			if (!output_term_literal(term->u.literal))
				return 0;
			break;
		case TYPE_TOKEN:
			if (!output_term_token(term->u.token))
				return 0;
		 	break;
		case TYPE_PROSE:
			if (!output_term_prose(term->u.prose))
				return 0;
			break;
		case TYPE_GROUP:
			if (!output_term_group(term->u.group))
				return 0;
			break;
	}

	fputs("}", stdout);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	fputs("{", stdout);
	output_string("$isa");
	fputs(":", stdout);
	output_string("alt");

	if (alt->invisible != 0) {
		fputs(",", stdout);
		output_string("invisible");
		fputs(":true", stdout);
	}

	if (alt->terms) {
		fputs(",", stdout);
		output_string("terms");
		fputs(":", stdout);
		fputs("[", stdout);

		for (term = alt->terms; term != NULL; term = term->next) {
			if (term != alt->terms)
					fputs(",", stdout);

			if (!output_term(term))
				return 0;
		}

		fputs("]", stdout);
	}
	fputs("}", stdout);

	return 1;
}

WARN_UNUSED_RESULT
static int
output_alts(const struct ast_alt *alts)
{
	const struct ast_alt *alt;

	fputs("[", stdout);

	for (alt = alts; alt != NULL; alt = alt->next) {
		if (alt != alts)
				fputs(",", stdout);

		if (!output_alt(alt))
			return 0;
	}

	fputs("]", stdout);
	return 1;
}

WARN_UNUSED_RESULT
static int
output_rule(const struct ast_rule *rule)
{
	fputs("{", stdout);
	output_string("$isa");
	fputs(":", stdout);
	output_string("rule");

	if (rule->name) {
		fputs(",", stdout);
		output_string("name");
		fputs(":", stdout);
		output_string(rule->name);
	}

	if (rule->alts) {
		fputs(",", stdout);
		output_string("alts");
		fputs(":", stdout);
		if (!output_alts(rule->alts))
			return 0;
	}

	fputs("}", stdout);

	return 1;
}

WARN_UNUSED_RESULT
int
json_output(const struct ast_rule *grammar)
{
	const struct ast_rule *rule;

	fputs("[", stdout);

	for (rule = grammar; rule != NULL; rule = rule->next) {
		if (rule != grammar)
				fputs(",", stdout);

		if (!output_rule(rule))
			return 0;
	}

	fputs("]\n", stdout);

	return 1;
}
