/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_TNODE_H
#define KGT_RRD_TNODE_H

struct node;

/*
 * Various combinations of two endpoints (corner pieces) for a line:
 *
 *       .              .
 *      -A---- node ----B-
 *       '              '
 */
enum tline {
	TLINE_A,
	TLINE_B,
	TLINE_C,
	TLINE_D,
	TLINE_E,
	TLINE_F,
	TLINE_G,
	TLINE_H,
	TLINE_I
};

/*
 * A list of vertical line segments:
 *
 *       A---- node ----B
 *       |              |
 *  .o --C---- node ----D--
 *       |              |
 *       E---- node ----F
 *       |              |
 *       G---- node ----H
 */
struct tnode_vlist {
	struct tnode **a;
	enum tline *b;
	size_t n;
	unsigned o; /* offset, in indicies */
};

/*
 * A list of horizontal line segments:
 *
 *     -- node -- node -- node --
 */
struct tnode_hlist {
	struct tnode **a;
	size_t n;
};

struct tnode {
	enum {
		TNODE_SKIP,
		TNODE_ARROW,
		TNODE_ELLIPSIS,
		TNODE_CI_LITERAL,
		TNODE_CS_LITERAL,
		TNODE_LABEL,
		TNODE_RULE,
		TNODE_VLIST,
		TNODE_HLIST
	} type;

	unsigned w;
	unsigned a; /* ascender  - height including and above the line  */
	unsigned d; /* descender - depth below the line */

	int rtl;

	union {
		const char *literal; /* TODO: point to ast_literal instead */
		const char *name;    /* TODO: point to ast_rule instead */
		const char *label;

		struct tnode_vlist vlist;
		struct tnode_hlist hlist;
	} u;
};

void
tnode_free(struct tnode *n);

struct tnode *
rrd_to_tnode(const struct node *node,
	void (*dim_string)(const char *s, unsigned *w, unsigned *a, unsigned *d));

#endif

