/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_ISO_EBNF_IO_H
#define KGT_ISO_EBNF_IO_H

#include "../compiler_specific.h"
#include "../parsing_error.h"
struct ast_rule;

/*
 * We don't mark FEATURE_AST_INVISIBLE as unsupported here, because this EBNF
 * is supposed to be a source format; it's not presentational.
 */

#define iso_ebnf_ast_unsupported (FEATURE_AST_CI_LITERAL | FEATURE_AST_PROSE | FEATURE_AST_BINARY)

struct ast_rule *
iso_ebnf_input(int (*f)(void *opaque), void *opaque, parsing_error_queue* errors);

WARN_UNUSED_RESULT
int
iso_ebnf_output(const struct ast_rule *grammar);

#endif

