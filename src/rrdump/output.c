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
node_walk(struct node **n, int depth, void *opaque);

static void
print_indent(FILE *f, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static int
visit_skip(struct node *n, struct node **np, int depth, void *opaque)
{
	FILE *f = opaque;

	(void) n;
	(void) np;

	print_indent(f, depth);
	fprintf(f, "SKIP\n");

	return 1;
}

static int
visit_name(struct node *n, struct node **np, int depth, void *opaque)
{
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "NAME: <%s>\n", n->u.name);

	return 1;
}

static int
visit_literal(struct node *n, struct node **np, int depth, void *opaque)
{
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "LITERAL: \"%s\"\n", n->u.literal);

	return 1;
}

static int
visit_alt(struct node *n, struct node **np, int depth, void *opaque)
{
	struct list **p;
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "ALT: [\n");
	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		if (!node_walk(&(*p)->node, depth + 1, f)) {
			return 0;
		}
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_seq(struct node *n, struct node **np, int depth, void *opaque)
{
	struct list **p;
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "SEQ: [\n");
	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		if (!node_walk(&(*p)->node, depth + 1, f)) {
			return 0;
		}
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_loop(struct node *n, struct node **np, int depth, void *opaque)
{
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "LOOP:\n");
	if (n->u.loop.forward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		if (!node_walk(&n->u.loop.forward, depth + 2, f)) {
			return 0;
		}
	}

	if (n->u.loop.backward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		if (!node_walk(&n->u.loop.backward, depth + 2, f)) {
			return 0;
		}
	}

	return 1;
}

static int
node_walk(struct node **n, int depth, void *opaque)
{
	struct node *node;

	assert(n != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:
		return visit_skip(node, n, depth, opaque);

	case NODE_LITERAL:
		return visit_literal(node, n, depth, opaque);

	case NODE_RULE:
		return visit_name(node, n, depth, opaque);

	case NODE_ALT:
		return visit_alt(node, n, depth, opaque);

	case NODE_SEQ:
		return visit_seq(node, n, depth, opaque);

	case NODE_LOOP:
		return visit_loop(node, n, depth, opaque);
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
			node_walk(&rrd, 1, stdout);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			node_walk(&rrd, 1, stdout);
			printf("\n");

			rrd_pretty_prefixes(&rrd);
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);
			rrd_pretty_collapse(&rrd);

			printf("%s: (after prettify)\n", p->name);
			node_walk(&rrd, 1, stdout);
			printf("\n");
		}

		node_free(rrd);
	}
}

