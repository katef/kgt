/*
 * Copyright 2021 John Scott
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_JSON_IO_H
#define KGT_JSON_IO_H

#include "../compiler_specific.h"

struct ast_rule;

#define json_ast_unsupported (FEATURE_AST_INVISIBLE)

WARN_UNUSED_RESULT
int
json_output(const struct ast_rule *grammar);

#endif
