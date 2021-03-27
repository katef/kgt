/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_BNF_IO_H
#define KGT_BNF_IO_H

#include "../compiler_specific.h"
#include "../parsing_error.h"
struct ast_rule;

#define bnf_ast_unsupported (FEATURE_AST_CI_LITERAL | FEATURE_AST_BINARY | FEATURE_AST_INVISIBLE)

struct ast_rule *
bnf_input(int (*f)(void *opaque), void *opaque, parsing_error_queue* errors);

WARN_UNUSED_RESULT
int
bnf_output(const struct ast_rule *grammar);

#endif

