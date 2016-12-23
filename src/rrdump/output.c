/* $Id$ */

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

static int
node_walk(FILE *f, struct node **n, int depth);

static void
print_indent(FILE *f, int n)
{
	int i;

	assert(f != NULL);

	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static int
visit_skip(FILE *f, struct node *n, struct node **np, int depth)
{
	(void) n;
	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "SKIP\n");

	return 1;
}

static int
visit_name(FILE *f, struct node *n, struct node **np, int depth)
{
	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "NAME: <%s>\n", n->u.name);

	return 1;
}

static int
visit_literal(FILE *f, struct node *n, struct node **np, int depth)
{
	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "LITERAL: \"%s\"\n", n->u.literal);

	return 1;
}

static int
visit_alt(FILE *f, struct node *n, struct node **np, int depth)
{
	struct list **p;

	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "ALT: [\n");
	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		if (!node_walk(f, &(*p)->node, depth + 1)) {
			return 0;
		}
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_seq(FILE *f, struct node *n, struct node **np, int depth)
{
	struct list **p;

	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "SEQ: [\n");
	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		if (!node_walk(f, &(*p)->node, depth + 1)) {
			return 0;
		}
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_loop(FILE *f, struct node *n, struct node **np, int depth)
{
	(void) np;

	assert(f != NULL);

	print_indent(f, depth);
	fprintf(f, "LOOP:\n");
	if (n->u.loop.forward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		if (!node_walk(f, &n->u.loop.forward, depth + 2)) {
			return 0;
		}
	}

	if (n->u.loop.backward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		if (!node_walk(f, &n->u.loop.backward, depth + 2)) {
			return 0;
		}
	}

	return 1;
}

static int
node_walk(FILE *f, struct node **n, int depth)
{
	struct node *node;

	assert(f != NULL);
	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:
		return visit_skip(f, node, n, depth);

	case NODE_LITERAL:
		return visit_literal(f, node, n, depth);

	case NODE_RULE:
		return visit_name(f, node, n, depth);

	case NODE_ALT:
		return visit_alt(f, node, n, depth);

	case NODE_SEQ:
		return visit_seq(f, node, n, depth);

	case NODE_LOOP:
		return visit_loop(f, node, n, depth);
	}

	return 1;
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
			node_walk(stdout, &rrd, 1);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			node_walk(stdout, &rrd, 1);
			printf("\n");

			rrd_pretty_prefixes(&rrd);
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);
			rrd_pretty_collapse(&rrd);

			printf("%s: (after prettify)\n", p->name);
			node_walk(stdout, &rrd, 1);
			printf("\n");
		}

		node_free(rrd);
	}
}

