/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Output a plaintext dump of the railroad tree
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../txt.h"
#include "../ast.h"
#include "../compiler_specific.h"

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

	if (n == NULL) {
		print_indent(f, depth);
		fprintf(f, "SKIP\n");
		return;
	}

	switch (n->type) {
		const struct list *p;

	case NODE_CI_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL%s: \"%.*s\"/i\n",
			n->invisible ? " (invisible)" : "",
			(int) n->u.literal.n, n->u.literal.p);

		break;

	case NODE_CS_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL%s: \"%.*s\"\n",
			n->invisible ? " (invisible)" : "",
			(int) n->u.literal.n, n->u.literal.p);

		break;

	case NODE_RULE:
		print_indent(f, depth);
		fprintf(f, "NAME%s: <%s>\n",
			n->invisible ? " (invisible)" : "",
			n->u.name);

		break;

	case NODE_PROSE:
		print_indent(f, depth);
		fprintf(f, "PROSE%s: ?%s?\n",
			n->invisible ? " (invisible)" : "",
			n->u.prose);

		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		print_indent(f, depth);
		fprintf(f, "%s%s: [\n",
			n->invisible ? " (invisible)" : "",
			n->type == NODE_ALT ? "ALT" : "ALT|SKIP");
		for (p = n->u.alt; p != NULL; p = p->next) {
			node_walk(f, p->node, depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case NODE_SEQ:
		print_indent(f, depth);
		fprintf(f, "SEQ%s: [\n",
			n->invisible ? " (invisible)" : "");
		for (p = n->u.seq; p != NULL; p = p->next) {
			node_walk(f, p->node, depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case NODE_LOOP:
		print_indent(f, depth);
		fprintf(f, "LOOP%s:\n",
			n->invisible ? " (invisible)" : "");

		if (n->u.loop.forward != NULL) {
			print_indent(f, depth + 1);
			fprintf(f, ".forward:\n");
			node_walk(f, n->u.loop.forward, depth + 2);
		}

		if (n->u.loop.backward != NULL) {
			print_indent(f, depth + 1);
			fprintf(f, ".backward:\n");
			node_walk(f, n->u.loop.backward, depth + 2);
		}

		break;
	}
}

WARN_UNUSED_RESULT
int
rrdump_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return 0;
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
	return 1;
}

