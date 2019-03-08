/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRTEXT_IO_H
#define KGT_RRTEXT_IO_H

struct ast_rule;

extern int prettify;

void
rrutf8_output(const struct ast_rule *);

void
rrtext_output(const struct ast_rule *);

#endif
