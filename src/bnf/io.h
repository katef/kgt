/* $Id$ */

#ifndef KGT_BNF_IO_H
#define KGT_BNF_IO_H

struct ast_rule;

struct ast_rule *
bnf_input(int (*f)(void *opaque), void *opaque);

void
bnf_output(struct ast_rule *grammar);

#endif

