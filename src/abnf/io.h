/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_ABNF_IO_H
#define KGT_ABNF_IO_H

#include "../compiler_specific.h"
#include "../parsing_error.h"
struct ast_rule;

/*
 * We don't mark FEATURE_AST_INVISIBLE as unsupported here, because ABNF
 * is supposed to be a source format; it's not presentational.
 */

struct ast_rule *
abnf_input(int (*f)(void *opaque), void *opaque, parsing_error_queue* errors);

WARN_UNUSED_RESULT
int
abnf_output(const struct ast_rule *grammar);

#endif

