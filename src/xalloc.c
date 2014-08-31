/* $Id$ */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "xalloc.h"

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

