/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Extended Backus-Naur Form Output, pretty-printed to HTML.
 *
 * This is my own made-up dialect. It's intended for ease
 * of human consumption (i.e. in documentation), rather than
 * to be parsed by machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../rrd/node.h"

#include "io.h"

extern const char *css_file;

WARN_UNUSED_RESULT
int
cat(const char *in, const char *indent);

static void output_alt(const struct ast_alt *alt);

/* TODO: centralise */
static int
xml_escputc(FILE *f, char c)
{
	const char *name;

	assert(f != NULL);

	switch (c) {
	case '&': return fputs("&amp;", f);
	case '<': return fputs("&lt;", f);
	case '>': return fputs("&gt;", f);

	case '\a': name = "BEL"; break;
	case '\b': name = "BS";  break;
	case '\f': name = "FF";  break;
	case '\n': name = "LF";  break;
	case '\r': name = "CR";  break;
	case '\t': name = "TAB"; break;
	case '\v': name = "VT";  break;

	default:
		if (!isprint((unsigned char) c)) {
			return fprintf(f, "&#x3008;<tspan class='hex'>%02X</tspan>&#x3009;", (unsigned char) c);
		}

		return fprintf(f, "%c", c);
	}

	return fprintf(f, "&#x3008;<tspan class='esc'>%s</tspan>&#x3009;", name);
}

static int
atomic(const struct ast_term *term)
{
	assert(term != NULL);

	switch (term->type) {
	case TYPE_EMPTY:
	case TYPE_RULE:
	case TYPE_CI_LITERAL:
	case TYPE_CS_LITERAL:
	case TYPE_TOKEN:
	case TYPE_PROSE:
		return 1;

	case TYPE_GROUP:
		if (term->u.group->next != NULL) {
			return 0;
		}

		if (term->u.group->terms->next != NULL) {
			return 0;
		}

		return atomic(term->u.group->terms);
	}

	assert(!"unreached");
}

static const char *
rep(unsigned min, unsigned max)
{
	if (min == 1 && max == 1) {
		/* no operator */
		return "\0";
	}

	if (min == 1 && max == 1) {
		return "()";
	}

	if (min == 0 && max == 1) {
		return "[]";
	}

	if (min == 0 && max == 0) {
		return "{}";
	}

	return "()";
}

static void
output_literal(const char *prefix, const struct txt *t)
{
	size_t i;

	assert(t != NULL);
	assert(t->p != NULL);

	printf("<tt class='literal %s'>&quot;", prefix);

	for (i = 0; i < t->n; i++) {
		xml_escputc(stdout, t->p[i]);
	}

	printf("&quot;</tt>");
}

static void
output_term(const struct ast_term *term)
{
	const char *r;

	assert(term != NULL);
	assert(!term->invisible);

	r = rep(term->min, term->max);

	if (!r[0] && !atomic(term)) {
		r = "()";
	}

	if (r[0]) {
		printf("<span class='rep'>%c</span> ", r[0]);
	}

	/* TODO: escaping */

	switch (term->type) {
	case TYPE_EMPTY:
		printf("<span class='empty'>&epsilon;</span>");
		break;

	case TYPE_RULE:
		printf("<a href='#%s' class='rule' data-min='%u' data-max='%u'>",
			term->u.rule->name, term->min, term->max);
		printf("%s", term->u.rule->name);
		printf("</a>");
		break;

	case TYPE_CI_LITERAL:
		output_literal("ci", &term->u.literal);
		break;

	case TYPE_CS_LITERAL:
		output_literal("cs", &term->u.literal);
		break;

	case TYPE_TOKEN:
		printf("<span class='token'>");
		printf("%s", term->u.token);
		printf("</span>");
		break;

	case TYPE_PROSE:
		printf("<span class='prose'>");
		printf("%s", term->u.prose);
		printf("</span>");
		break;

	case TYPE_GROUP: {
			const struct ast_alt *alt;

			for (alt = term->u.group; alt != NULL; alt = alt->next) {
				output_alt(alt);

				if (alt->next != NULL) {
					printf("<span class='pipe'> | </span>");
				}
			}
		}

		break;
	}

	if (r[0]) {
		printf(" <span class='rep'>%c</span>", r[1]);
	}

	if (term->max > 1) {
		printf("<sub class='rep'>{%u, %u}</sub>", term->min, term->max);
	}
}

static void
output_alt(const struct ast_alt *alt)
{
	const struct ast_term *term;

	assert(alt != NULL);
	assert(!alt->invisible);

	for (term = alt->terms; term != NULL; term = term->next) {
		printf("<span class='alt'>");
		output_term(term);
		printf("</span>\n");

		if (term->next != NULL) {
			printf("<span class='cat'> </span>");
		}
	}
}

static void
output_rule(const struct ast_rule *rule)
{
	const struct ast_alt *alt;

	printf("  <dl class='bnf'>\n");

	printf("    <dt>");
	printf("<a name='%s'>", rule->name);
	printf("%s", rule->name);
	printf("</a>:");
	printf("</dt>\n");

	printf("    <dd>");

	for (alt = rule->alts; alt != NULL; alt = alt->next) {
		if (alt != rule->alts) {
			printf("<span class='pipe'> | </span>");
		}

		printf("\n");
		printf("      ");
		output_alt(alt);

		if (alt->next != NULL) {
			printf("<br/>\n");
			printf("      ");
		}
	}

	printf("    </dd>\n");

	printf("  </dl>\n");
	printf("\n");
}

WARN_UNUSED_RESULT
static int
output(const struct ast_rule *grammar, int xml)
{
	const struct ast_rule *p;

	printf(" <head>\n");
	if (xml) {
		printf("  <meta charset='UTF-8'/>\n");
	}

	printf("  <style>\n");

	printf("    dl.bnf span.token {\n");
	printf("    	text-transform: uppercase;\n");
	printf("    }\n");
	printf("    \n");
	printf("    dl.bnf span.cat {\n");
	printf("    	margin-right: 0.5ex;\n");
	printf("    }\n");
	printf("    \n");
	printf("    dl.bnf dd > span.pipe {\n");
	printf("    	float: left;\n");
	printf("    	width: 1ex;\n");
	printf("    	margin-left: -1.8ex;\n");
	printf("    	text-align: right;\n");
	printf("    	padding-right: .8ex; /* about the width of a space */\n");
	printf("    }\n");
	printf("    \n");
	printf("    dl.bnf dt {\n");
	printf("    	display: block;\n");
	printf("    	min-width: 8em;\n");
	printf("    	padding-right: 1em;\n");
	printf("    }\n");
	printf("    \n");
	printf("    dl.bnf a.rule {\n");
	printf("    	text-decoration: none;\n");
	printf("    }\n");
	printf("    \n");
	printf("    dl.bnf a.rule:hover {\n");
	printf("    	text-decoration: underline;\n");
	printf("    }\n");
	printf("    \n");
	printf("    /* page stuff */\n");
	printf("    dl.bnf { margin: 2em 4em; }\n");
	printf("    dl.bnf dt { margin: 0.25em 0; }\n");
	printf("    dl.bnf dd { margin-left: 2em; }\n");

	if (css_file != NULL) {
		if (!cat(css_file, "    ")) {
			return 0;
		}
	}

	printf("  </style>\n");

	printf(" </head>\n");

	printf(" <body>\n");

	for (p = grammar; p != NULL; p = p->next) {
		output_rule(p);
	}

	printf(" </body>\n");
	return 1;
}

WARN_UNUSED_RESULT
int
ebnf_html5_output(const struct ast_rule *grammar)
{
	printf("<!DOCTYPE html>\n");
	printf("<html>\n");
	printf("\n");

	if (!output(grammar, 0)) {
		return 0;
	}

	printf("</html>\n");
	return 1;
}

WARN_UNUSED_RESULT
int
ebnf_xhtml5_output(const struct ast_rule *grammar)
{
	printf("<?xml version='1.0' encoding='utf-8'?>\n");
	printf("<!DOCTYPE html>\n");
	printf("<html xml:lang='en' lang='en'\n");
	printf("  xmlns='http://www.w3.org/1999/xhtml'\n");
	printf("  xmlns:xlink='http://www.w3.org/1999/xlink'>\n");
	printf("\n");

	if (!output(grammar, 1)) {
		return 0;
	}

	printf("</html>\n");
	return 1;
}

