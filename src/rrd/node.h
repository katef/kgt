/* $Id$ */

#ifndef KGT_RRD_NODE_H
#define KGT_RRD_NODE_H

struct node;

void
node_free(struct node *);

struct node *
node_create_skip(void);

struct node *
node_create_terminal(const char *terminal);

struct node *
node_create_identifier(const char *identifier);

struct node *
node_create_choice(struct node *choice);

struct node *
node_create_sequence(struct node *sequence);

struct node *
node_create_loop(struct node *forward, struct node *backward);

void
node_collapse(struct node **n);

#endif

