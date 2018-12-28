/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_NODE_H
#define KGT_RRD_NODE_H

struct node {
	enum {
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
};

void
node_free(struct node *);

struct node *
node_create_literal(const char *literal);

struct node *
node_create_name(const char *name);

struct node *
node_create_alt(struct list *alt);

struct node *
node_create_seq(struct list *seq);

struct node *
node_create_loop(struct node *forward, struct node *backward);

void
node_make_seq(struct node **n);

int
node_compare(const struct node *a, const struct node *b);

void
loop_flip(struct node *n);

#endif

