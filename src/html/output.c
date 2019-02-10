/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Railroad Diagram HTML5 Output
 *
 * Output a HTML page of inlined SVG diagrams
 */

#define _BSD_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"
#include "../rrd/tnode.h"

#include "io.h"

/* XXX */
extern struct dim svg_dim;
void svg_render_rule(const struct tnode *node, const char *base);
void svg_defs(void);

void
html_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("<DOCTYPE html>\n");
	printf("<html>\n");
	printf("\n");

	printf(" <head>\n");

	printf("  <style>\n");
	printf("      rect, line, path { stroke-width: 1.5px; stroke: black; fill: transparent; }\n");
	printf("      rect, line, path { stroke-linecap: square; stroke-linejoin: rounded; }\n");
	printf("      path { fill: transparent; }\n");
	printf("      text.literal { font-family: monospace; }\n");
	printf("      a { fill: blue; }\n");
	printf("      a:hover rect { fill: aliceblue; }\n");
	printf("      h2 { font-size: inherit; font-weight: inherit; }\n");
	printf("      line.ellipsis { stroke-dasharray: 4; }\n");
	printf("      path.arrow.rtl { marker-mid: url(#rrd:arrow-rtl); }\n");
	printf("      path.arrow.ltr { marker-mid: url(#rrd:arrow-ltr); }\n");
	printf("  </style>\n");
	printf("\n");

	printf(" <svg height='0' width='0' style='float: left'>\n"); /* XXX: why does this take space? */
	svg_defs();
	printf(" </svg>\n");
	printf("\n");

	printf(" </head>\n");
	printf("\n");

	printf(" <body>\n");

	for (p = grammar; p; p = p->next) {
		struct tnode *tnode;
		struct node *rrd;
		unsigned h, w;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		tnode = rrd_to_tnode(rrd, &svg_dim);

		node_free(rrd);

		printf(" <section>\n");
		printf("  <h2><a name='%s'>%s:</a></h2>\n",
			p->name, p->name);

		h = (tnode->a + tnode->d + 2) * 10;
		w = (tnode->w + 7) * 10;

		printf("  <svg height='%u' width='%u' viewBox='-20 -10 %u %u'>\n",
			h, w, w, h);
		svg_render_rule(tnode, "");
		printf("  </svg>\n");

		printf(" </section>\n");
		printf("\n");

		tnode_free(tnode);
	}

	printf(" </body>\n");
	printf("</html>\n");
}

