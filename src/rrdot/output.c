/* $Id$ */

/*
 * Abstract Railroad Diagram tree dump to Graphivz
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"

#include "io.h"

void
rrdot_output(struct ast_production *grammar)
{
	struct ast_production *p;

	printf("digraph G {\n");
	printf("\tnode [ shape = record, style = rounded ];\n");
	printf("\tedge [ dir = none ];\n");

	for (p = grammar; p; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);
		}

        printf("\t\"%s/%p\" [ shape = plaintext, label = \"%s\" ];\n",
			p->name, (void *) p, p->name,
			p->name);

		rrd_print_dot(p->name, p, &rrd);

		node_free(rrd);
	}

	printf("};\n");
	printf("\n");
}

