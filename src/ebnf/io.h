/* $Id$ */

#ifndef KGT_EBNF_IO_H
#define KGT_EBNF_IO_H

struct ast_production;

struct ast_production *
ebnf_input(int (*f)(void *opaque), void *opaque);

void
ebnf_output(struct ast_production *grammar);

#endif

