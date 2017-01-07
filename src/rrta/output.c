/* $Id$ */

/*
 * Tab Atkins Jr. Railroad Diagram Output.
 * See https://github.com/tabatkins/railroad-diagrams
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
		fprintf(f, "Skip()");

		break;

	case NODE_LITERAL:
		print_indent(f, depth);
		fprintf(f, "Terminal(\"%s\")", (*n)->u.literal); /* XXX: escape */

		break;

	case NODE_RULE:
		print_indent(f, depth);
		fprintf(f, "NonTerminal(\"%s\")", (*n)->u.name); /* XXX: escape */

		break;

	case NODE_ALT:
		print_indent(f, depth);
		fprintf(f, "Choice(0,\n"); /* TODO: find a way to indicate the "normal" choice */

		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(f, &(*p)->node, depth + 1);
			if ((**p).next != NULL) {
				fprintf(f, ",");
				fprintf(f, "\n");
			}
		}
		fprintf(f, ")");

		break;

	case NODE_SEQ:
		print_indent(f, depth);
		fprintf(f, "Sequence(\n");
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(f, &(*p)->node, depth + 1);
			if ((**p).next != NULL) {
				fprintf(f, ",");
				fprintf(f, "\n");
			}
		}
		fprintf(f, ")");

		break;

	case NODE_LOOP:
		print_indent(f, depth);
		fprintf(f, "%s(\n", &(*n)->u.loop.min == 0 ? "ZeroOrMore" : "OneOrMore");

		node_walk(f, &(*n)->u.loop.forward, depth + 1);
		fprintf(f, ",\n");

		node_walk(f, &(*n)->u.loop.backward, depth + 1);
		fprintf(f, ")");

		break;
	}
}

void
rrta_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

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

		printf("add('%s', Diagram(\n", p->name); /* XXX: escape */

		node_walk(stdout, &rrd, 1);
		printf("));\n");
		printf("\n");

		node_free(rrd);
	}
}

