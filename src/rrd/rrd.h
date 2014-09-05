/* $Id$ */

#ifndef KGT_RRD_H
#define KGT_RRD_H

#include "../ast.h"

enum leaf_type {
	LEAF_TERMINAL,
	LEAF_IDENTIFIER
};

enum list_type {
	LIST_CHOICE,
	LIST_SEQUENCE
};

struct box_size {
	int w;
	int h;
};

struct node {
	enum {
		NODE_SKIP,
		NODE_LEAF,
		NODE_LIST,
		NODE_LOOP
	} type;

	union {
		struct {
			enum leaf_type type;
			const char *text;
		} leaf;

		struct {
			enum list_type type;
			struct node *list;
		} list;

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
	int (*visit_identifier)(struct node *, struct node **, int, void *);
	int (*visit_terminal  )(struct node *, struct node **, int, void *);
	int (*visit_choice    )(struct node *, struct node **, int, void *);
	int (*visit_sequence  )(struct node *, struct node **, int, void *);
	int (*visit_loop      )(struct node *, struct node **, int, void *);
};

int node_walk(struct node **, const struct node_walker *, int, void *);
int node_walk_list(struct node **, const struct node_walker *, int, void *);

struct node *ast_to_rrd(struct ast_production *);

#endif
