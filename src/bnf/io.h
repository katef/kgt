/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_BNF_IO_H
#define KGT_BNF_IO_H

struct ast_rule;

#define bnf_ast_unsupported FEATURE_AST_CI_LITERAL

struct ast_rule *
bnf_input(int (*f)(void *opaque), void *opaque);

void
bnf_output(const struct ast_rule *grammar);

#endif

