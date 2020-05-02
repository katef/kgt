/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_REWRITE_H
#define KGT_REWRITE_H

struct ast_rule;

void
rewrite_ci_literals(struct ast_rule *g);

void
rewrite_invisible(struct ast_rule *g);

#endif

