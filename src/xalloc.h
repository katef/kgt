/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_XALLOC_H
#define KGT_XALLOC_H

#include <stddef.h>

void *xmalloc(size_t size);
char *xstrdup(const char *s);
void xerror(const char *msg, ...);

#endif

