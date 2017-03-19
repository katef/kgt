/*
 * Output a plaintext dump of the railroad tree
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
		fprintf(f, "    ");
	}
}

static void
node_walk(FILE *f, const struct node *n, int depth)
{
	assert(f != NULL);
	assert(n != NULL);

	switch (n->type) {
		const struct list *p;

	case NODE_SKIP:
		print_indent(f, depth);
		fprintf(f, "SKIP\n");

		break;

	case NODE_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL: \"%s\"\n", n->u.literal);

		break;

	case NODE_RULE:
		print_indent(f, depth);
		fprintf(f, "NAME: <%s>\n", n->u.name);

		break;

	case NODE_ALT:
		print_indent(f, depth);
		fprintf(f, "ALT: [\n");
		for (p = n->u.alt; p != NULL; p = p->next) {
			node_walk(f, p->node, depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case NODE_SEQ:
		print_indent(f, depth);
		fprintf(f, "SEQ: [\n");
		for (p = n->u.seq; p != NULL; p = p->next) {
			node_walk(f, p->node, depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case NODE_LOOP:
		print_indent(f, depth);
		fprintf(f, "LOOP:\n");

		if (n->u.loop.forward->type != NODE_SKIP) {
			print_indent(f, depth + 1);
			fprintf(f, ".forward:\n");
			node_walk(f, n->u.loop.forward, depth + 2);
		}

		if (n->u.loop.backward->type != NODE_SKIP) {
			print_indent(f, depth + 1);
			fprintf(f, ".backward:\n");
			node_walk(f, n->u.loop.backward, depth + 2);
		}

		break;
	}
}

void
rrdump_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

		if (!prettify) {
			printf("%s:\n", p->name);
			node_walk(stdout, rrd, 1);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			node_walk(stdout, rrd, 1);
			printf("\n");

			rrd_pretty(&rrd);

			printf("%s: (after prettify)\n", p->name);
			node_walk(stdout, rrd, 1);
			printf("\n");
		}

		node_free(rrd);
	}
}

