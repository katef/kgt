/* $Id$ */

#ifndef KGT_RRD_H
#define KGT_RRD_H

#include "../ast.h"

struct box_size {
	int w;
	int h;
};

struct node {
	enum {
		NODE_SKIP,
		NODE_LITERAL,
		NODE_RULE,
		NODE_ALT,
		NODE_SEQ,
		NODE_LOOP
	} type;

	union {
		const char *literal; /* TODO: point to ast_literal instead */
		const char *name;    /* TODO: point to ast_rule instead */

		struct list *alt;
		struct list *seq;

		struct {
			struct node *forward;
			struct node *backward;
			unsigned int min;
			unsigned int max;
		} loop;
	} u;

	int y;
	struct box_size size;
};

struct node *ast_to_rrd(const struct ast_rule *);

#endif
