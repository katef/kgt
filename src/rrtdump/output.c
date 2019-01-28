/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Output a plaintext dump of the railroad tnode tree
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/list.h"

#include "../rrtext/tnode.h"

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
print_coords(FILE *f, const struct tnode *n)
{
	fprintf(f, " w=%u a=%u d=%u", n->w, n->a, n->d);
}

static void
tnode_walk(FILE *f, const struct tnode *n, int depth)
{
	assert(f != NULL);
	assert(n != NULL);

	switch (n->type) {
		size_t i;

	case TNODE_SKIP:
		print_indent(f, depth);
		fprintf(f, "SKIP");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_ELLIPSIS:
		print_indent(f, depth);
		fprintf(f, "ELLIPSIS");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL");
		print_coords(f, n);
		fprintf(f, ": \"%s\"\n", n->u.literal);

		break;

	case TNODE_RULE:
		print_indent(f, depth);
		fprintf(f, "NAME");
		print_coords(f, n);
		fprintf(f, ": <%s>\n", n->u.name);

		break;

	case TNODE_ALT:
		print_indent(f, depth);
		fprintf(f, "ALT");
		print_coords(f, n);
		fprintf(f, " o=%u", n->o); /* XXX: belongs in alt union */
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.alt.n; i++) {
			tnode_walk(f, n->u.alt.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case TNODE_SEQ:
		print_indent(f, depth);
		fprintf(f, "SEQ");
		print_coords(f, n);
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.seq.n; i++) {
			tnode_walk(f, n->u.seq.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case TNODE_LOOP:
		print_indent(f, depth);
		fprintf(f, "LOOP");
		print_coords(f, n);
		fprintf(f, ":\n");

		if (n->u.loop.forward != NULL) {
			print_indent(f, depth + 1);
			fprintf(f, ".forward:\n");
			tnode_walk(f, n->u.loop.forward, depth + 2);
		}

		if (n->u.loop.backward != NULL) {
			print_indent(f, depth + 1);
			fprintf(f, ".backward:\n");
			tnode_walk(f, n->u.loop.backward, depth + 2);
		}

		break;
	}
}

static void
dim_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = strlen(s); /* monospace */
	*a = 0;
	*d = 1;
}

void
rrtdump_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		struct tnode *tnode;
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return;
		}

		tnode = rrd_to_tnode(rrd, dim_string);

		if (!prettify) {
			printf("%s:\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");

			tnode_free(tnode);

			rrd_pretty(&rrd);

			tnode = rrd_to_tnode(rrd, dim_string);

			printf("%s: (after prettify)\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");
		}

		tnode_free(tnode);
		node_free(rrd);
	}
}

