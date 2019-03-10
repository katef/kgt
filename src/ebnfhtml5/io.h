/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_EBNFHTML5_IO_H
#define KGT_EBNFHTML5_IO_H

struct ast_rule;

void
ebnf_html5_output(const struct ast_rule *grammar);

void
ebnf_xhtml5_output(const struct ast_rule *grammar);

#endif

