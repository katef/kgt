/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_REWRITE_H
#define KGT_RRD_REWRITE_H

struct ast_rule;
struct node;

void
rewrite_rrd_ci_literals(struct node *n);

#endif

