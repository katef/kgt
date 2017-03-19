#ifndef KGT_WSN_IO_H
#define KGT_WSN_IO_H

struct ast_rule;

struct ast_rule *
wsn_input(int (*f)(void *opaque), void *opaque);

void
wsn_output(const struct ast_rule *grammar);

#endif

