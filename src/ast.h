/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_AST_H
#define KGT_AST_H

struct ast_alt;

/*
 * A term is a sequential list of items within an alt. Each item may be
 * a terminal literal, a production rule name, or a group of terms.
 *
 * A term may be repeated a number of times, as in the EBNF x * 3 construct.
 *
 * a + b + c ;
 */
struct ast_term {
	enum {
		TYPE_EMPTY,
		TYPE_RULE,
		TYPE_LITERAL,
		TYPE_TOKEN,
		TYPE_GROUP
	} type;

	union {
		struct ast_rule *rule;
		const char *literal;
		const char *token;
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
 * A grammar is a list of production rules. Each rule maps a name onto a list
 * of alternatives:
 *
 * name1 := alt1 | alt2 | alt3 ;
 * name2 := alt1 ;
 */
struct ast_rule {
	const char *name;
	struct ast_alt *alts;

	struct ast_rule *next;
};

struct ast_term *
ast_make_empty_term(void);

struct ast_term *
ast_make_rule_term(struct ast_rule *rule);

struct ast_term *
ast_make_char_term(char c);

struct ast_term *
ast_make_literal_term(const char *literal);

struct ast_term *
ast_make_token_term(const char *token);

struct ast_term *
ast_make_group_term(struct ast_alt *group);

struct ast_alt *
ast_make_alt(struct ast_term *terms);

struct ast_rule *
ast_make_rule(const char *name, struct ast_alt *alts);

struct ast_rule *
ast_find_rule(const struct ast_rule *grammar, const char *name);

void
ast_free_rule(struct ast_rule *rule);

#endif

