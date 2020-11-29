/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * parcon.railroad DSL Railroad Diagram Output.
 * http://www.opengroove.org/parcon/parcon-tutorial.html
 *
 * Output adapted from the following example:
 * https://github.com/javawizard/everything/blob/4912dc91f7fe458d2324177d678618c22961ee99/projects/misc/sd-example.py
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/rewrite.h"
#include "../rrd/node.h"
#include "../rrd/list.h"

#include "io.h"

static void
print_indent(FILE *f, int n)
{
	int i;

	assert(f != NULL);

	for (i = 0; i < n; i++) {
		fprintf(f, "  ");
	}
}

/* python */
static int
escputc(int c, FILE *f)
{
	size_t i;

	const struct {
		int c;
		const char *s;
	} a[] = {
		{ '\\', "\\\\" },
		{ '\"', "\\\"" },

		{ '\a', "\\a"  },
		{ '\b', "\\b"  },
		{ '\f', "\\f"  },
		{ '\n', "\\n"  },
		{ '\r', "\\r"  },
		{ '\t', "\\t"  },
		{ '\v', "\\v"  }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (a[i].c == c) {
			return fputs(a[i].s, f);
		}
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "\\%03o", (unsigned char) c);
	}

	return fprintf(f, "%c", c);
}

/* TODO: centralise, maybe with callback */
static int
escputs(const char *s, FILE *f)
{
	const char *p;
	int r, n;

	assert(s != NULL);
	assert(f != NULL);

	n = 0;

	for (p = s; *p != '\0'; p++) {
		r = escputc(*p, f);
		if (r < 0) {
			return -1;
		}

		n += r;
	}

	return n;
}

/* TODO: centralise */
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
print_comment(FILE *f, int depth, const char *fmt, ...)
{
	va_list ap;

	assert(f != NULL);
	assert(fmt != NULL);

	va_start(ap, fmt);

	print_indent(f, depth);
	fprintf(f, "comment(\"");
	vfprintf(f, fmt, ap);
	fprintf(f, "\")");

	va_end(ap);
}

WARN_UNUSED_RESULT
static int
node_walk(FILE *f, const struct node *n, int depth)
{
	assert(f != NULL);

	if (n == NULL) {
		print_indent(f, depth);
		fprintf(f, "Nothing()");

		return 1;
	}

	assert(!n->invisible);

	switch (n->type) {
		const struct list *p;

	case NODE_CI_LITERAL:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case NODE_CS_LITERAL:
		print_indent(f, depth);
		fprintf(f, "text(\"");
		escputt(&n->u.literal, f);
		fprintf(f, "\")");

		break;

	case NODE_RULE:
		print_indent(f, depth);
		fprintf(f, "production(\"");
		escputs(n->u.name, f);
		fprintf(f, "\")");

		break;

	case NODE_PROSE:
		fprintf(stderr, "unimplemented\n");
		return 0;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		print_indent(f, depth);
		fprintf(f, "Or(\n");

		if (n->type == NODE_ALT_SKIPPABLE) {
			print_indent(f, depth + 1);
			fprintf(f, "Nothing(),\n");
		}

		for (p = n->u.alt; p != NULL; p = p->next) {
			if (!node_walk(f, p->node, depth + 1))
				return 0;
			if (p->next != NULL) {
				fprintf(f, ",");
				fprintf(f, "\n");
			}
		}
		fprintf(f, "\n");

		print_indent(f, depth);
		fprintf(f, ")");

		break;

	case NODE_SEQ:
		print_indent(f, depth);
		fprintf(f, "Then(\n");
		for (p = n->u.seq; p != NULL; p = p->next) {
			if (!node_walk(f, p->node, depth + 1))
				return 0;
			if (p->next != NULL) {
				fprintf(f, ",");
				fprintf(f, "\n");
			}
		}
		fprintf(f, "\n");

		print_indent(f, depth);
		fprintf(f, ")");

		break;

	case NODE_LOOP:
		print_indent(f, depth);
		fprintf(f, "Loop(\n");

		if (!node_walk(f, n->u.loop.forward, depth + 1))
			return 0;
		fprintf(f, ",\n");

		if (n->u.loop.max == 1 && n->u.loop.min == 1) {
			print_comment(f, depth + 1, "(exactly once)");
			assert(n->u.loop.backward == NULL);
		} else if (n->u.loop.max == 0 && n->u.loop.min > 0) {
			print_comment(f, depth + 1, "(at least %d times)", n->u.loop.min);
			assert(n->u.loop.backward == NULL);
		} else if (n->u.loop.max > 0 && n->u.loop.min == 0) {
			print_comment(f, depth + 1, "(up to %d times)", n->u.loop.max);
			assert(n->u.loop.backward == NULL);
		} else if (n->u.loop.max > 0 && n->u.loop.min == n->u.loop.max) {
			print_comment(f, depth + 1, "(%d times)", n->u.loop.max);
			assert(n->u.loop.backward == NULL);
		} else if (n->u.loop.max > 1 && n->u.loop.min > 1) {
			print_comment(f, depth + 1, "(%d-%d times)", n->u.loop.min, n->u.loop.max);
			assert(n->u.loop.backward == NULL);
		} else {
			if (!node_walk(f, n->u.loop.backward, depth))
				return 0;
		}

		fprintf(f, "\n");

		print_indent(f, depth);
		fprintf(f, ")");
		break;
	}
	return 1;
}

WARN_UNUSED_RESULT
int
rrparcon_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("#!/usr/bin/env python\n");
	printf("# -*- coding: utf-8 -*-\n");
	printf("\n");

	printf("import sys\n");
	printf("from collections import OrderedDict\n");
	printf("\n");
	printf("from parcon.railroad import Then, Or, Token, Loop, Bullet, Nothing\n");
	printf("from parcon.railroad import PRODUCTION, TEXT, DESCRIPTION\n");
	printf("import parcon.railroad.raildraw\n");
	printf("\n");

	printf("comment    = lambda t: Token(DESCRIPTION, t)\n");
	printf("production = lambda t: Token(PRODUCTION, t)\n");
	printf("text       = lambda t: Token(TEXT, t)\n");
	printf("\n");

	printf("productions = OrderedDict([\n");

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return 0;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		/* TODO: pass in unsupported bitmap */
		if (!rewrite_rrd_ci_literals(rrd))
			return 0;

		printf("  (\n");
		printf("    \"");
		escputs(p->name, stdout);
		printf("\",\n");
		printf("    Then(\n");
		printf("      Bullet(),\n");

		if (!node_walk(stdout, rrd, 3))
			return 0;
		printf(",");
		printf("\n");

		printf("      Bullet()\n");
		printf("    )\n");
		printf("  )");
		if (p->next != NULL) {
			printf(",");
		}
		printf("\n");

		node_free(rrd);
	}

	printf("])\n");
	printf("\n");

	printf("options = {\n");

	printf("  \"raildraw_arrow_width\":       8,\n"); /* 0 also looks nice */
	printf("  \"raildraw_arrow_height\":      8,\n");
	printf("  \"raildraw_then_before_arrow\": 8,\n");
	printf("  \"raildraw_then_after_arrow\":  0,\n");
	printf("  \"raildraw_or_before\":         8,\n");
	printf("  \"raildraw_or_after\":          8,\n");

	printf("  \"raildraw_token_padding\":     2,\n");
	printf("  \"raildraw_bullet_radius\":     4,\n");
	printf("  \"raildraw_or_radius\":        10,\n");
	printf("  \"raildraw_line_size\":         2,\n");

	printf("  \"raildraw_title_before\":     20,\n");
	printf("  \"raildraw_title_after\":      30,\n");
	printf("  \"raildraw_scale\":             1\n");

	printf("}\n");
	printf("\n");

	printf("# parcon.railroad.raildraw.draw_to_image(sys.argv[1], productions, options, sys.argv[2], True)\n");
	printf("parcon.railroad.raildraw.draw_to_png(productions, options, sys.argv[2], True)\n");
	printf("\n");
	return 1;
}

