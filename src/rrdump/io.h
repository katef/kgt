/* $Id$ */

#ifndef KGT_RRDUMP_IO_H
#define KGT_RRDUMP_IO_H

struct ast_production;

extern int prettify;

void
rrdump_output(struct ast_production *);

#endif
