/* $Id$ */

#ifndef KGT_RRD_NODE_H
#define KGT_RRD_NODE_H

struct node;

void
node_free(struct node *);

struct node *
node_create_skip(void);

struct node *
node_create_leaf(enum leaf_type type, const char *text);

struct node *
node_create_list(enum list_type type, struct node *list);

struct node *
node_create_loop(struct node *forward, struct node *backward);

#endif

