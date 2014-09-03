/* $Id$ */

#ifndef KGT_WSN_IO_H
#define KGT_WSN_IO_H

struct ast_production;

struct ast_production *
wsn_input(int (*f)(void *opaque), void *opaque);

void
wsn_output(struct ast_production *grammar);

#endif

