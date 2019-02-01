/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRTA_IO_H
#define KGT_RRTA_IO_H

struct ast_rule;

#define rrta_rrd_unsupported 0

extern int prettify;

void
rrta_output(const struct ast_rule *grammar);

#endif

