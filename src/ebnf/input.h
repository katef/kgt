/* $Id$ */

#ifndef KGT_EBNF_INPUT_H
#define KGT_EBNF_INPUT_H

struct ast_production *
ebnf_input(int (*f)(void *opaque), void *opaque);

#endif

