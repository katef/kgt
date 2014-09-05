/* $Id$ */

#ifndef KGT_RRD_H
#define KGT_RRD_H

#include "../ast.h"

enum node_type {
	NT_SKIP,
	NT_LEAF,
	NT_LIST,
	NT_LOOP
};

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
	struct node *next;
	enum node_type type;
	struct box_size size;
	int y;
};

struct node_leaf {
	struct node node;
	enum leaf_type type;
	const char *text;
};

struct node_list {
	struct node node;
	enum list_type type;
	struct node *list;
};

struct node_loop {
	struct node node;
	struct node *forward;
	struct node *backward;
};

void *node_create(enum node_type);
void node_free(struct node *);

void node_collapse(struct node **);


/* node traversal - visit functions are passed a pointer to the pointer that ties
 * the visited node into the tree.
 * they are free to replace the node they visited via said pointer. cf. beautify
 */
struct node_walker {
	int (*visit_nothing   )(struct node *,		struct node **, int, void *);
	int (*visit_identifier)(struct node_leaf *, struct node **, int, void *);
	int (*visit_terminal  )(struct node_leaf *, struct node **, int, void *);
	int (*visit_choice	  )(struct node_list *, struct node **, int, void *);
	int (*visit_sequence  )(struct node_list *, struct node **, int, void *);
	int (*visit_loop	  )(struct node_loop *, struct node **, int, void *);
};

int node_walk(struct node **, const struct node_walker *, int, void *);
int node_walk_list(struct node **, const struct node_walker *, int, void *);

int ast_to_rrd(struct ast_production *, struct node **);

#endif
