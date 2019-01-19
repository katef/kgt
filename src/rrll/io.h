/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRLL_IO_H
#define KGT_RRLL_IO_H

struct ast_rule;

extern int prettify;

void
rrll_output(const struct ast_rule *grammar);

#endif

