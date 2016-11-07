/* $Id$ */

#ifndef KGT_RRD_NODE_H
#define KGT_RRD_NODE_H

struct node;

void
node_free(struct node *);

struct node *
node_create_skip(void);

struct node *
node_create_literal(const char *literal);

struct node *
node_create_name(const char *name);

struct node *
node_create_alt(struct node *alt);

struct node *
node_create_sequence(struct node *sequence);

struct node *
node_create_loop(struct node *forward, struct node *backward);

void
node_collapse(struct node **n);

int
node_compare(struct node *a, struct node *b);

#endif

