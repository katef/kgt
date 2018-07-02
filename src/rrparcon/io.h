/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRPARCON_IO_H
#define KGT_RRPARCON_IO_H

struct ast_rule;

extern int prettify;

void
rrparcon_output(const struct ast_rule *);

#endif