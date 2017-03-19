/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_H
#define KGT_RRD_H

struct ast_rule;
struct node;

struct node *ast_to_rrd(const struct ast_rule *);

#endif
