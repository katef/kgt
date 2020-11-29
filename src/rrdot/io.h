/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRDOT_IO_H
#define KGT_RRDOT_IO_H

#include "../compiler_specific.h"

struct ast_rule;

extern int prettify;

WARN_UNUSED_RESULT
int
rrdot_output(const struct ast_rule *);

#endif
