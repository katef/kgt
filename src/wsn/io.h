/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_WSN_IO_H
#define KGT_WSN_IO_H

struct ast_rule;

#define wsn_ast_unsupported FEATURE_AST_CI_LITERAL

struct ast_rule *
wsn_input(int (*f)(void *opaque), void *opaque);

void
wsn_output(const struct ast_rule *grammar);

#endif

