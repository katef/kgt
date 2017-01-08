/* $Id$ */

/*
 * Abstract Railroad Diagram tree dump to Graphivz
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/list.h"

#include "io.h"

static int
escputc(int c, FILE *f)
{
	size_t i;

	const struct {
		int c;
		const char *s;
	} a[] = {
		{ '\\', "\\\\"   },
		{ '&',  "&amp;"  },
		{ '\"', "&quot;" },
		{ '<',  "&#x3C;" },
		{ '>',  "&#x3E;" }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (a[i].c == c) {
			return fputs(a[i].s, f);
		}
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "&#x%X;", (unsigned char) c);
	}

	return putc(c, f);
}

static int
escputs(const char *s, FILE *f)
{
	const char *p;
	int r;

	assert(f != NULL);
	assert(s != NULL);

	for (p = s; *p != '\0'; p++) {
		r = escputc(*p, f);
		if (r < 0) {
			return -1;
		}
	}

	return 0;
}

static void
rrd_print_dot(const char *prefix, const void *parent, const char *port,
	const struct node *node)
{
	switch (node->type) {
		const struct list *p;

	case NODE_ALT:
		printf("\t{ rank = same;\n");
		for (p = node->u.alt; p != NULL; p = p->next) {
			printf("\t\t\"%s/%p\";\n", prefix, (void *) p->node);
		}
		printf("\t};\n");
		break;

	case NODE_SEQ:
		printf("\t{ rank = same;\n");
		for (p = node->u.seq; p != NULL; p = p->next) {
			printf("\t\t\"%s/%p\";\n", prefix, (void *) p->node);
		}
		printf("\t};\n");
		break;

	default:
		break;
	}

	printf("\t\"%s/%p\"%s -> \"%s/%p\";\n",
		prefix, parent, port,
		prefix, (void *) node);

	printf("\t\"%s/%p\" [ ",
		prefix, (void *) node);

	switch (node->type) {
	case NODE_SKIP:
		printf("label = \"&epsilon;\"");
		break;

	case NODE_LITERAL:
		printf("style = filled, shape = box, label = \"\\\"");
		escputs(node->u.literal, stdout);
		printf("\\\"\"");
		break;

	case NODE_RULE:
		printf("label = \"\\<");
		escputs(node->u.name, stdout);
		printf("\\>\"");
		break;

	case NODE_ALT:
		printf("label = \"ALT\"");
		break;

	case NODE_SEQ:
		printf("label = \"SEQ\"");
		break;

	case NODE_LOOP:
		printf("label = \"<b> &larr;|LOOP|<f> &rarr;\""); /* TODO: utf */
		break;

	default:
		printf("label = \"?\", color = red");
	}

	printf(" ];\n");

	switch (node->type) {
		const struct list *p;

	case NODE_ALT:
		for (p = node->u.alt; p != NULL; p = p->next) {
			rrd_print_dot(prefix, node, "", p->node);
		}
		break;

	case NODE_SEQ:
		for (p = node->u.seq; p != NULL; p = p->next) {
			rrd_print_dot(prefix, node, "", p->node);
		}
		break;

	case NODE_LOOP:
		rrd_print_dot(prefix, node, ":f", node->u.loop.forward);
		rrd_print_dot(prefix, node, ":b", node->u.loop.backward);
		break;

	default:
		break;
	}
}

void
rrdot_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	printf("digraph G {\n");
	printf("\tnode [ shape = record, style = rounded ];\n");
	printf("\tedge [ dir = none ];\n");

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

        printf("\t\"%s/%p\" [ shape = plaintext, label = \"%s\" ];\n",
			p->name, (void *) p,
			p->name);

		rrd_print_dot(p->name, p, "", rrd);

		node_free(rrd);
	}

	printf("}\n");
	printf("\n");
}

