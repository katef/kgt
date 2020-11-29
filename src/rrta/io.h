/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRTA_IO_H
#define KGT_RRTA_IO_H

struct ast_rule;

#define rrta_ast_unsupported (FEATURE_AST_BINARY | FEATURE_AST_INVISIBLE)
#define rrta_rrd_unsupported FEATURE_RRD_CI_LITERAL

extern int prettify;

WARN_UNUSED_RESULT
int
rrta_output(const struct ast_rule *grammar);

#endif

