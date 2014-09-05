/* $Id$ */

/*
 * Railroad Diagram Output
 *
 * Output a plaintext diagram of the abstract representation of railroads
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
rrtext_output(struct ast_production *grammar)
{
	struct ast_production *p;

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

		printf("%s:\n", p->name);
		rrd_print_text(&rrd);
		printf("\n");

		node_free(rrd);
	}
}

