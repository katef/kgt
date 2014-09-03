/* $Id$ */

#ifndef KGT_BNF_INPUT_H
#define KGT_BNF_INPUT_H

struct ast_production *
bnf_input(int (*f)(void *opaque), void *opaque);

#endif

