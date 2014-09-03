/* $Id$ */

/*
 * Railroad Diagram Output
 * Spew out a text description of the abstract representation of railroads
 * TODO: add more output formats (read: ones that actually draw diagrams)
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../ast.h"

#include "rrd.h"
#include "beautify.h"
#include "render.h"

static void print_indent(FILE *f, int n) {
	int i;
	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static struct node_walker rrd_print;

static int visit_nothing(struct node *n, struct node **np, int depth, void *arg) {
	FILE *f = arg;
	(void) n;
	(void) np;
	print_indent(f, depth);
	fprintf(f, "NT_NOTHING\n");
	return 1;
}

static int visit_leaf(struct node_leaf *n, struct node **np, int depth, void *arg) {
	FILE *f = arg;
	(void) np;
	print_indent(f, depth);
	if (n->type == LEAF_IDENTIFIER)
		fprintf(f, "NT_LEAF(IDENTIFIER): %s\n", n->text);
	else
		fprintf(f, "NT_LEAF(TERMINAL): \"%s\"\n", n->text);
	return 1;
}

static int visit_list(struct node_list *n, struct node **np, int depth, void *arg) {
	FILE *f = arg;
	(void) np;
	print_indent(f, depth);
	fprintf(f, "NT_LIST(%s): [\n", n->type == LIST_CHOICE ? "CHOICE" : "SEQUENCE");
	if (!node_walk_list(&n->list, &rrd_print, depth + 1, arg))
		return 0;
	print_indent(f, depth);
	fprintf(f, "]\n");
	return 1;
}

static int visit_loop(struct node_loop *n, struct node **np, int depth, void *arg) {
	FILE *f = arg;
	(void) np;
	print_indent(f, depth);
	fprintf(f, "NT_LOOP:\n");
	if (n->forward->type != NT_NOTHING) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		if (!node_walk_list(&n->forward, &rrd_print, depth + 2, arg))
			return 0;
	}
	if (n->backward->type != NT_NOTHING) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		if (!node_walk_list(&n->backward, &rrd_print, depth + 2, arg))
			return 0;
	}
	return 1;
}

static struct node_walker rrd_print = {
	visit_nothing,
	visit_leaf, visit_leaf,
	visit_list, visit_list,
	visit_loop
};

static void print_repr(struct node **n) {
	node_walk(n, &rrd_print, 1, stdout);
}

void rrd_output(struct ast_production *grammar, int beautify) {
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		struct node *rrd;
		assert(ast_to_rrd(p, &rrd) && "AST transformation failed somehow");

		if (beautify)
			rrd_beautify_all(&rrd);
		printf("%s:\n", p->name);
		rrd_render(&rrd);
		printf("\n");

		node_free(rrd);
	}
}
