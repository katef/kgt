/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRDUMP_IO_H
#define KGT_RRDUMP_IO_H

#include "../compiler_specific.h"

struct ast_rule;

extern int prettify;

WARN_UNUSED_RESULT
int
rrdump_output(const struct ast_rule *);

#endif
