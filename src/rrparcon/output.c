/* $Id$ */

/*
 * parcon.railroad DSL Railroad Diagram Output.
 * http://www.opengroove.org/parcon/parcon-tutorial.html
 *
 * Output adapted from the following example:
 * https://github.com/javawizard/parcon/blob/master/parcon/railroad/raildraw.py
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
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

static void
node_walk(FILE *f, struct node **n, int depth)
{
	assert(f != NULL);
	assert(n != NULL);

	switch ((*n)->type) {
		struct list **p;

	case NODE_SKIP:
		print_indent(f, depth);
		fprintf(f, "Nothing()");

		break;

	case NODE_LITERAL:
		print_indent(f, depth);
		fprintf(f, "text(\"%s\")", (*n)->u.literal); /* XXX: escape */

		break;

	case NODE_RULE:
		print_indent(f, depth);
		fprintf(f, "production(\"%s\")", (*n)->u.name); /* XXX: escape */

		break;

	case NODE_ALT:
		print_indent(f, depth);
		fprintf(f, "Or(\n");
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(f, &(*p)->node, depth + 1);
			if ((**p).next != NULL) {
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
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(f, &(*p)->node, depth + 1);
			if ((**p).next != NULL) {
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

		node_walk(f, &(*n)->u.loop.forward, depth + 1);
		fprintf(f, ",\n");

		node_walk(f, &(*n)->u.loop.backward, depth + 1);
		fprintf(f, "\n");

		print_indent(f, depth);
		fprintf(f, ")");
		break;
	}
}

void
rrparcon_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("#!/usr/bin/env python\n");
	printf("\n");

	printf("import sys\n");
	printf("from collections import OrderedDict\n");
	printf("\n");
	printf("from parcon.railroad import Then, Or, Token, Loop, Bullet, Nothing\n");
	printf("from parcon.railroad import PRODUCTION, TEXT\n");
	printf("import parcon.railroad.raildraw\n");
	printf("\n");

	printf("production = lambda t: Token(PRODUCTION, t)\n");
	printf("text = lambda t: Token(TEXT, t)\n");
	printf("\n");

	printf("productions = OrderedDict([\n");

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		printf("  (\n");
		printf("    \"%s\",\n", p->name); /* XXX: escape */
		printf("    Then(\n");
		printf("      Bullet(),\n");

		node_walk(stdout, &rrd, 3);
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
	printf("  \"raildraw_title_before\":20,\n");
	printf("  \"raildraw_title_after\": 30,\n");
	printf("  \"raildraw_scale\": 0.7\n");
	printf("}\n");
	printf("\n");

	printf("# parcon.railroad.raildraw.draw_to_image(sys.argv[1], productions, options, sys.argv[2], True)\n");
	printf("parcon.railroad.raildraw.draw_to_png(productions, options, sys.argv[2], True)\n");
	printf("\n");
}

