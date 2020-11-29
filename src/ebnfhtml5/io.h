/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_EBNFHTML5_IO_H
#define KGT_EBNFHTML5_IO_H

#include "../compiler_specific.h"

struct ast_rule;

/*
 * We mark FEATURE_AST_INVISIBLE as unsupported here, because this EBNF
 * is supposed to be a presentational format.
 */
#define ebnf_html5_ast_unsupported (FEATURE_AST_INVISIBLE)

WARN_UNUSED_RESULT
int
ebnf_html5_output(const struct ast_rule *grammar);

WARN_UNUSED_RESULT
int
ebnf_xhtml5_output(const struct ast_rule *grammar);

#endif

