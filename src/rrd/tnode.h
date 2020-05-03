/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_TNODE_H
#define KGT_RRD_TNODE_H

struct node;
struct txt;

/*
 * Various combinations of two endpoints (corner pieces) for a line:
 *
 *       .              .
 *      -A---- node ----B-
 *       '              '
 */
enum tline {
	TLINE_A, TLINE_a,
	TLINE_B,
	TLINE_C, TLINE_c,
	TLINE_D, TLINE_d,
	TLINE_E,
	TLINE_F,
	TLINE_G, TLINE_g,
	TLINE_H, TLINE_h,
	TLINE_I, TLINE_i,
	TLINE_J
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
		TNODE_RTL_ARROW,
		TNODE_LTR_ARROW,
		TNODE_ELLIPSIS,
		TNODE_CI_LITERAL,
		TNODE_CS_LITERAL,
		TNODE_COMMENT,
		TNODE_PROSE,
		TNODE_RULE,
		TNODE_VLIST,
		TNODE_HLIST
	} type;

	/* in abstract rrd units */
	unsigned w;
	unsigned a; /* ascender  - height including and above the line  */
	unsigned d; /* descender - depth below the line */

	union {
		struct txt literal; /* TODO: point to ast_literal instead */
		const char *name;   /* TODO: point to ast_rule instead */
		const char *prose;

		struct {
			const char *s;
			const struct tnode *tnode;
		} comment;

		struct tnode_vlist vlist;
		struct tnode_hlist hlist;
	} u;
};

struct dim {
	void (*literal_txt)(const struct txt *t, unsigned *w, unsigned *a, unsigned *d);
	void (*rule_string)(const char *s, unsigned *w, unsigned *a, unsigned *d);
	unsigned literal_padding;
	unsigned rule_padding;
	unsigned prose_padding;
	unsigned comment_height;
	unsigned ci_marker;
	unsigned ellipsis_depth;
};

void
tnode_free(struct tnode *n);

struct tnode *
rrd_to_tnode(const struct node *node, const struct dim *dim);

#endif

