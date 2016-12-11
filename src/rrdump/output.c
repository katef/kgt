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

#include "io.h"

static int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *opaque);

static void
print_indent(FILE *f, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static struct node_walker rrd_print;

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
	struct node **p;
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "ALT: [\n");
	for (p = &n->u.alt; *p != NULL; p = &(**p).next) {
		if (!node_walk(p, &rrd_print, depth + 1, f)) {
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
	struct node **p;
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "SEQ: [\n");
	for (p = &n->u.seq; *p != NULL; p = &(**p).next) {
		if (!node_walk(p, &rrd_print, depth + 1, f)) {
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
	struct node **p;
	FILE *f = opaque;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "LOOP:\n");
	if (n->u.loop.forward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		for (p = &n->u.loop.forward; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, &rrd_print, depth + 2, f)) {
				return 0;
			}
		}
	}

	if (n->u.loop.backward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		for (p = &n->u.loop.backward; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, &rrd_print, depth + 2, f)) {
				return 0;
			}
		}
	}

	return 1;
}

static struct node_walker rrd_print = {
	visit_skip,
	visit_name, visit_literal,
	visit_alt,  visit_seq,
	visit_loop
};

static int
node_walk(struct node **n, const struct node_walker *ws, int depth, void *opaque)
{
	int (*f)(struct node *, struct node **, int, void *);
	struct node *node;

	assert(n != NULL);
	assert(ws != NULL);

	node = *n;

	switch (node->type) {
	case NODE_SKIP:    f = ws->visit_skip;    break;
	case NODE_LITERAL: f = ws->visit_literal; break;
	case NODE_RULE:    f = ws->visit_name;    break;
	case NODE_ALT:     f = ws->visit_alt;     break;
	case NODE_SEQ:     f = ws->visit_seq;     break;
	case NODE_LOOP:    f = ws->visit_loop;    break;
	}

	if (f != NULL) {
		return f(node, n, depth, opaque);
	}

	switch (node->type) {
		struct node **p;

	case NODE_ALT:
		for (p = &node->u.alt; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, ws, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_SEQ:
		for (p = &node->u.seq; *p != NULL; p = &(**p).next) {
			if (!node_walk(p, ws, depth + 1, opaque)) {
				return 0;
			}
		}

		break;

	case NODE_LOOP:
		if (!node_walk(&node->u.loop.forward, ws, depth + 1, opaque)) {
			return 0;
		}

		if (!node_walk(&node->u.loop.backward, ws, depth + 1, opaque)) {
			return 0;
		}

		break;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		break;
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
			node_walk(&rrd, &rrd_print, 1, stdout);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			node_walk(&rrd, &rrd_print, 1, stdout);
			printf("\n");

			rrd_pretty_prefixes(&rrd);
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);

			printf("%s: (after prettify)\n", p->name);
			node_walk(&rrd, &rrd_print, 1, stdout);
			printf("\n");
		}

		node_free(rrd);
	}
}

