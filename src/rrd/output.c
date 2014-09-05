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

#include "io.h"
#include "rrd.h"
#include "pretty.h"
#include "print.h"
#include "node.h"

int prettify = 1;

void
rrd_output(struct ast_production *grammar)
{
	struct ast_production *p;

	for (p = grammar; p; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

/* XXX:
rrd_print_dump(&rrd);
*/

		if (prettify) {
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);
		}

/* XXX:
rrd_print_dump(&rrd);
*/

		printf("%s:\n", p->name);
		rrd_print_text(&rrd);
		printf("\n");

		node_free(rrd);
	}
}
