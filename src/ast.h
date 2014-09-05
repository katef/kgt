/* $Id$ */

#ifndef KGT_AST_H
#define KGT_AST_H

struct ast_alt;

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
		const char *name;	/* production name; TODO: point to ast_production instead */
		const char *literal;
		struct ast_alt *group;
	} u;

	unsigned int min;
	unsigned int max; /* false (0) for unlimited */

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

