/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_ISO_EBNF_IO_H
#define KGT_ISO_EBNF_IO_H

struct ast_rule;

struct ast_rule *
iso_ebnf_input(int (*f)(void *opaque), void *opaque);

void
iso_ebnf_output(const struct ast_rule *grammar);

#endif

