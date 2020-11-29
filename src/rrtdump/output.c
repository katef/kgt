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

#include "../txt.h"
#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/list.h"
#include "../rrd/tnode.h"
#include "../compiler_specific.h"

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

	case TNODE_RTL_ARROW:
		print_indent(f, depth);
		fprintf(f, "RTL_ARROW");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_LTR_ARROW:
		print_indent(f, depth);
		fprintf(f, "LTR_ARROW");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_ELLIPSIS:
		print_indent(f, depth);
		fprintf(f, "ELLIPSIS");
		print_coords(f, n);
		fprintf(f, "\n");

		break;

	case TNODE_CI_LITERAL:
	case TNODE_CS_LITERAL:
		print_indent(f, depth);
		fprintf(f, "LITERAL");
		print_coords(f, n);
		fprintf(f, ": \"%.*s\"%s\n", (int) n->u.literal.n, n->u.literal.p,
			n->type == TNODE_CI_LITERAL ? "/i" : "");

		break;

	case TNODE_PROSE:
		print_indent(f, depth);
		fprintf(f, "PROSE");
		print_coords(f, n);
		fprintf(f, ": %s\n", n->u.prose);

		break;

	case TNODE_COMMENT:
		print_indent(f, depth);
		fprintf(f, "COMMENT");
		print_coords(f, n);
		fprintf(f, " \"%s\"", n->u.comment.s);
		fprintf(f, ": (\n");
		tnode_walk(f, n->u.comment.tnode, depth + 1);
		print_indent(f, depth);
		fprintf(f, ")\n");

		break;

	case TNODE_RULE:
		print_indent(f, depth);
		fprintf(f, "RULE");
		print_coords(f, n);
		fprintf(f, ": <%s>\n", n->u.name);

		break;

	case TNODE_VLIST:
		print_indent(f, depth);
		fprintf(f, "VLIST");
		print_coords(f, n);
		fprintf(f, " o=%u", n->u.vlist.o); /* XXX: belongs in alt union */
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.vlist.n; i++) {
			/* TODO: show tline for each alt */
			tnode_walk(f, n->u.vlist.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;

	case TNODE_HLIST:
		print_indent(f, depth);
		fprintf(f, "HLIST");
		print_coords(f, n);
		fprintf(f, ": [\n");
		for (i = 0; i < n->u.hlist.n; i++) {
			tnode_walk(f, n->u.hlist.a[i], depth + 1);
		}
		print_indent(f, depth);
		fprintf(f, "]\n");

		break;
	}
}

static void
dim_mono_txt(const struct txt *t, unsigned *w, unsigned *a, unsigned *d)
{
	assert(t != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = t->n;
	*a = 0;
	*d = 1;
}

static void
dim_mono_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = strlen(s);
	*a = 0;
	*d = 1;
}

WARN_UNUSED_RESULT
int
rrtdump_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	struct dim dim = {
		dim_mono_txt,
		dim_mono_string,
		0,
		0,
		0,
		0,
		0,
		0
	};

	for (p = grammar; p != NULL; p = p->next) {
		struct tnode *tnode;
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return 0;
		}

		tnode = rrd_to_tnode(rrd, &dim);

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

			tnode = rrd_to_tnode(rrd, &dim);

			printf("%s: (after prettify)\n", p->name);
			tnode_walk(stdout, tnode, 1);
			printf("\n");
		}

		tnode_free(tnode);
		node_free(rrd);
	}
	return 1;
}

