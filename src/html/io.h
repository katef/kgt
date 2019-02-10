/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_HTML_IO_H
#define KGT_HTML_IO_H

struct ast_rule;

extern int prettify;

void
html_output(const struct ast_rule *);

void
xhtml_output(const struct ast_rule *);

#endif
