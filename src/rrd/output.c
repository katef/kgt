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

void print_indent(FILE *f, int n) {
	int i;
	for (i = 0; i < n; i++) {
		fprintf(f, "    ");
	}
}

static struct node_walker rrd_print;

static int visit_nothing(void *arg, int depth, struct node *n) {
	FILE *f = arg;
	(void) n;
	print_indent(f, depth);
	fprintf(f, "NT_NOTHING\n");
	return 1;
}

static int visit_leaf(void *arg, int depth, struct node_leaf *n) {
	FILE *f = arg;
	print_indent(f, depth);
	if (n->type == LEAF_IDENTIFIER)
		fprintf(f, "NT_LEAF(IDENTIFIER): %s\n", n->text);
	else
		fprintf(f, "NT_LEAF(TERMINAL): \"%s\"\n", n->text);
	return 1;
}

static int visit_list(void *arg, int depth, struct node_list *n) {
	FILE *f = arg;
	print_indent(f, depth);
	fprintf(f, "NT_LIST(%s): [\n", n->type == LIST_CHOICE ? "CHOICE" : "SEQUENCE");
	if (!node_walk_list(n->list, &rrd_print, depth + 1, arg))
		return 0;
	print_indent(f, depth);
	fprintf(f, "]\n");
	return 1;
}

static int visit_loop(void *arg, int depth, struct node_loop *n) {
	FILE *f = arg;
	print_indent(f, depth);
	fprintf(f, "NT_LOOP:\n");
	if (n->forward->type != NT_NOTHING) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		if (!node_walk_list(n->forward, &rrd_print, depth + 2, arg))
			return 0;
	}
	if (n->backward->type != NT_NOTHING) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		if (!node_walk_list(n->backward, &rrd_print, depth + 2, arg))
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

void rrd_output(struct ast_production *grammar) {
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		struct node *rrd;
		assert(ast_to_rrd(p, &rrd) && "AST transformation failed somehow");
		printf("%s:\n", p->name);
		rrd_beautify(rrd);
		node_walk(rrd, &rrd_print, 1, stdout);
		node_free(rrd);
	}
}

