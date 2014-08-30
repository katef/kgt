/* $Id$ */

#ifndef KGT_AST_H
#define KGT_AST_H

struct ast_term;

/*
 * A group collates a sub-list of terms within an alt, for the pupose of
 * specifying their repetitivity:
 *
 * { a + b + c }
 */
struct ast_group {
	enum {
		KLEENE_STAR,    /* * */
		KLEENE_CROSS,   /* + */
		KLEENE_GROUP,	/* grouping for alts (not actually a kleene operator) */
		KLEENE_OPTIONAL /* ? (nor is this!) */
	} kleene;

	struct ast_alt *alts;
};

/*
 * A term is a sequential list of items within an alt. Each item may be
 * a terminal literal, a production name, or a group of terms.
 *
 * A term may be repeated a number of times, as in the EBNF x * 3 construct.
 *
 * a + b + c ;
 */
struct ast_term {
	enum {
		TYPE_EMPTY,
		TYPE_PRODUCTION,
		TYPE_TERMINAL,
		TYPE_GROUP
	} type;

	union {
		const char *name;	/* production name */
		const char *literal;
		struct ast_group *group;
	} u;

	unsigned int repeat;

	struct ast_term *next;
};

/*
 * An alternative is one of several choices:
 *
 * a | b | c
 */
struct ast_alt {
	struct ast_term *terms;
	/* TODO: struct ast_term *negs; - negative terms here */

	struct ast_alt *next;
};

/*
 * A grammar is a list of productions. Each production maps a name onto a list
 * of alternatives:
 *
 * name1 := alt1 | alt2 | alt3 ;
 * name2 := alt1 ;
 */
struct ast_production {
	const char *name;
	struct ast_alt *alts;

	struct ast_production *next;
};

#endif

