/* $Id$ */

#ifndef KGT_RRDOT_IO_H
#define KGT_RRDOT_IO_H

struct ast_rule;

extern int prettify;

void
rrdot_output(const struct ast_rule *);

#endif
