/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "xalloc.h"
#include "txt.h"

void *
xmalloc(size_t size)
{
	void *new;

	new = malloc(size);
	if (new == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return new;
}

char *
xstrdup(const char *s)
{
	char *new;

	new = xmalloc(strlen(s) + 1);
	return strcpy(new, s);
}

struct txt
xtxtdup(const struct txt *t)
{
	struct txt new;

	assert(t != NULL);
	assert(t->p != NULL);

	new.n = t->n;
	new.p = xmalloc(new.n);

	memcpy((void *) new.p, t->p, new.n);

	return new;
}

void
xerror(const char *msg, ...)
{
	va_list ap;

	fprintf(stderr, "kgt: ");

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(EXIT_FAILURE);
}

