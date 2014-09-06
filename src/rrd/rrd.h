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
		NODE_TERMINAL,
		NODE_RULE,
		NODE_CHOICE,
		NODE_SEQUENCE,
		NODE_LOOP
	} type;

	union {
		const char *terminal; /* TODO: point to ast_terminal instead */
		const char *name;     /* TODO: point to ast_rule instead */

		struct node *choice;
		struct node *sequence;

		struct {
			struct node *forward;
			struct node *backward;
		} loop;
	} u;

	int y;
	struct box_size size;

	struct node *next;
};

/* node traversal - visit functions are passed a pointer to the pointer that ties
 * the visited node into the tree.
 * they are free to replace the node they visited via said pointer. cf. beautify
 */
struct node_walker {
	int (*visit_skip      )(struct node *, struct node **, int, void *);
	int (*visit_name)(struct node *, struct node **, int, void *);
	int (*visit_terminal  )(struct node *, struct node **, int, void *);
	int (*visit_choice    )(struct node *, struct node **, int, void *);
	int (*visit_sequence  )(struct node *, struct node **, int, void *);
	int (*visit_loop      )(struct node *, struct node **, int, void *);
};

int node_walk(struct node **, const struct node_walker *, int, void *);
int node_walk_list(struct node **, const struct node_walker *, int, void *);

struct node *ast_to_rrd(struct ast_rule *);

#endif
