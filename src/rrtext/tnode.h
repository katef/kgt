/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRTEXT_TNODE_H
#define KGT_RRTEXT_TNODE_H

struct node;

enum tnode_looptype {
	TNODE_LOOP_ONCE,
	TNODE_LOOP_ATLEAST,
	TNODE_LOOP_UPTO,
	TNODE_LOOP_EXACTLY,
	TNODE_LOOP_BETWEEN
};

struct tlist {
	struct tnode **a;
	size_t n;
};

struct tnode {
	enum {
		TNODE_LITERAL,
		TNODE_RULE,
		TNODE_ALT,
		TNODE_ALT_SKIPPABLE,
		TNODE_SEQ,
		TNODE_LOOP
	} type;

	unsigned w;
	unsigned y;
	unsigned h;

	union {
		const char *literal; /* TODO: point to ast_literal instead */
		const char *name;    /* TODO: point to ast_rule instead */

		struct tlist alt;
		struct tlist seq;

		struct {
			enum tnode_looptype looptype;
			struct tnode *forward;
			struct tnode *backward;
			unsigned int min;
			unsigned int max;
		} loop;
	} u;
};

void
tnode_free(struct tnode *n);

struct tnode *
rrd_to_tnode(const struct node *node);

/* XXX */
size_t
loop_label(const struct tnode *loop, char *s);

#endif

