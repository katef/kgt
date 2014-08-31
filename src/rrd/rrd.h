/* $Id$ */

#ifndef KGT_RRD_H
#define KGT_RRD_H

#include "../ast.h"

enum node_type {
	NT_NOTHING,
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

struct node {
	struct node *next;
	enum node_type type;
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
void *node_duplicate(struct node *);
void node_free(struct node *);

struct node_walker {
	int (*visit_nothing   )(void *, int, struct node *);
	int (*visit_identifier)(void *, int, struct node_leaf *);
	int (*visit_terminal  )(void *, int, struct node_leaf *);
	int (*visit_choice    )(void *, int, struct node_list *);
	int (*visit_sequence  )(void *, int, struct node_list *);
	int (*visit_loop      )(void *, int, struct node_loop *);
};

int node_walk(struct node *, const struct node_walker *, int, void *);
int node_walk_list(struct node *, const struct node_walker *, int, void *);

int ast_to_rrd(struct ast_production *, struct node **);

#endif
