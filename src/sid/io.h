/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_SID_IO_H
#define KGT_SID_IO_H

#include "../compiler_specific.h"

struct ast_rule;

#define sid_ast_unsupported (FEATURE_AST_CI_LITERAL | FEATURE_AST_BINARY | FEATURE_AST_INVISIBLE)

WARN_UNUSED_RESULT
int
sid_output(const struct ast_rule *grammar);

#endif

