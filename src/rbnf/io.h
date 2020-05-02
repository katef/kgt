/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RBNF_IO_H
#define KGT_RBNF_IO_H

struct ast_rule;

#define rbnf_ast_unsupported (FEATURE_AST_CI_LITERAL | FEATURE_AST_BINARY | FEATURE_AST_INVISIBLE)

struct ast_rule *
rbnf_input(int (*f)(void *opaque), void *opaque);

void
rbnf_output(const struct ast_rule *grammar);

#endif

