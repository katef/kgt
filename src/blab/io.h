/*
 * Copyright 2018 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_BLAB_IO_H
#define KGT_BLAB_IO_H

#include "../compiler_specific.h"

struct ast_rule;

#define blab_ast_unsupported (FEATURE_AST_INVISIBLE)

WARN_UNUSED_RESULT
int
blab_output(const struct ast_rule *grammar);

#endif

