/* $Id$ */

#ifndef KGT_RRDUMP_IO_H
#define KGT_RRDUMP_IO_H

struct ast_rule;

extern int prettify;

void
rrdump_output(const struct ast_rule *);

#endif
