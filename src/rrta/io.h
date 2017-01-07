/* $Id$ */

#ifndef KGT_RRTA_IO_H
#define KGT_RRTA_IO_H

struct ast_rule;

extern int prettify;

void
rrta_output(const struct ast_rule *grammar);

#endif

