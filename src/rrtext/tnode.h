/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRTEXT_TNODE_H
#define KGT_RRTEXT_TNODE_H

struct node;

enum tline {
	TLINE_A,
	TLINE_B,
	TLINE_C,
	TLINE_D,
	TLINE_E,
	TLINE_F,
	TLINE_G,
	TLINE_H
};

struct tlist_alt {
	struct tnode **a;
	enum tline *b;
	size_t n;
};

struct tlist_seq {
	struct tnode **a;
	size_t n;
};

struct tnode {
	enum {
		TNODE_SKIP,
		TNODE_ELLIPSIS,
		TNODE_CI_LITERAL,
		TNODE_CS_LITERAL,
		TNODE_LABEL,
		TNODE_RULE,
		TNODE_ALT,
		TNODE_SEQ
	} type;

	unsigned w;
	unsigned a; /* ascender  - height including and above the line  */
	unsigned d; /* descender - depth below the line */

	int rtl;

	unsigned o; /* offset, in indicies; XXX: applies for alts only. should be inside union */

	union {
		const char *literal; /* TODO: point to ast_literal instead */
		const char *name;    /* TODO: point to ast_rule instead */
		const char *label;

		struct tlist_alt alt;
		struct tlist_seq seq;
	} u;
};

void
tnode_free(struct tnode *n);

struct tnode *
rrd_to_tnode(const struct node *node,
	void (*dim_string)(const char *s, unsigned *w, unsigned *a, unsigned *d));

#endif

