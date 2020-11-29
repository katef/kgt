/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_REWRITE_H
#define KGT_RRD_REWRITE_H

#include "../compiler_specific.h"

struct ast_rule;
struct node;

WARN_UNUSED_RESULT
int
rewrite_rrd_ci_literals(struct node *n);

#endif

