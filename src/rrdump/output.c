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
			rrd_print_dump(&rrd);
			printf("\n");
		} else {
			printf("%s: (before prettify)\n", p->name);
			rrd_print_dump(&rrd);
			printf("\n");

			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);

			printf("%s: (after prettify)\n", p->name);
			rrd_print_dump(&rrd);
			printf("\n");
		}

		node_free(rrd);
	}
}

