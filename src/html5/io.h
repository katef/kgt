/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_HTML5_IO_H
#define KGT_HTML5_IO_H

#include "../compiler_specific.h"

struct ast_rule;

extern int prettify;

WARN_UNUSED_RESULT
int
html5_output(const struct ast_rule *);

WARN_UNUSED_RESULT
int
xhtml5_output(const struct ast_rule *);

#endif
