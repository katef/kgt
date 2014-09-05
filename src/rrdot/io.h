/* $Id$ */

#ifndef KGT_RRDOT_IO_H
#define KGT_RRDOT_IO_H

struct ast_production;

extern int prettify;

void
rrdot_output(struct ast_production *);

#endif
