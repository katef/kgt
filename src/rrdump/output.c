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
visit_skip(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) n;
	(void) np;

	print_indent(f, depth);
	fprintf(f, "SKIP\n");

	return 1;
}

static int
visit_identifier(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "IDENTIFIER: <%s>\n", n->u.identifier);

	return 1;
}

static int
visit_terminal(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "TERMINAL: \"%s\"\n", n->u.terminal);

	return 1;
}

static int
visit_choice(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "CHOICE: [\n");
	if (!node_walk_list(&n->u.choice, &rrd_print, depth + 1, arg)) {
		return 0;
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "SEQUENCE: [\n");
	if (!node_walk_list(&n->u.sequence, &rrd_print, depth + 1, arg)) {
		return 0;
	}
	print_indent(f, depth);
	fprintf(f, "]\n");

	return 1;
}

static int
visit_loop(struct node *n, struct node **np, int depth, void *arg)
{
	FILE *f = arg;

	(void) np;

	print_indent(f, depth);
	fprintf(f, "LOOP:\n");
	if (n->u.loop.forward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".forward:\n");
		if (!node_walk_list(&n->u.loop.forward, &rrd_print, depth + 2, arg)) {
			return 0;
		}
	}

	if (n->u.loop.backward->type != NODE_SKIP) {
		print_indent(f, depth + 1);
		fprintf(f, ".backward:\n");
		if (!node_walk_list(&n->u.loop.backward, &rrd_print, depth + 2, arg)) {
			return 0;
		}
	}

	return 1;
}

static struct node_walker rrd_print = {
	visit_skip,
	visit_identifier, visit_terminal,
	visit_choice,     visit_sequence,
	visit_loop
};

void
rrdump_output(struct ast_production *grammar)
{
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
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

