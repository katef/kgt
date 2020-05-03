/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRLL_IO_H
#define KGT_RRLL_IO_H

struct ast_rule;

#define rrll_ast_unsupported (FEATURE_AST_BINARY | FEATURE_AST_INVISIBLE)
#define rrll_rrd_unsupported FEATURE_RRD_CI_LITERAL

extern int prettify;

void
rrll_output(const struct ast_rule *grammar);

#endif

