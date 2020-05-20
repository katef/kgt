/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_SVG_IO_H
#define KGT_SVG_IO_H

struct ast_rule;

extern int debug;
extern int prettify;

void
svg_output(const struct ast_rule *);

#endif
