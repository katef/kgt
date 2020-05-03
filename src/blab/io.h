/*
 * Copyright 2018 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_BLAB_IO_H
#define KGT_BLAB_IO_H

struct ast_rule;

#define blab_ast_unsupported (FEATURE_AST_INVISIBLE)

void
blab_output(const struct ast_rule *grammar);

#endif

