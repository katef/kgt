/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_REWRITE_H
#define KGT_REWRITE_H

#include "compiler_specific.h"

struct ast_rule;

WARN_UNUSED_RESULT
int
rewrite_ci_literals(struct ast_rule *g);

void
rewrite_invisible(struct ast_rule *g);

#endif

