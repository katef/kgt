/* $Id$ */

/*
 * Main support interfaces.
 */

#ifndef KGT_MAIN_H
#define KGT_MAIN_H

#include <stddef.h>

int act_read_token(void);

void *xmalloc(size_t size);
char *xstrdup(const char *s);
void xerror(const char *msg, ...);

#endif

