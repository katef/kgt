/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>
#include <ctype.h>

#include "txt.h"

int
txtcasecmp(const struct txt *t1, const struct txt *t2)
{
	size_t i;

	assert(t1 != NULL);
	assert(t2 != NULL);

	if (t1->n < t2->n) {
		return -1;
	}

	if (t1->n > t2->n) {
		return +1;
	}

	for (i = 0; i < t1->n; i++) {
		if (tolower((unsigned char) t1->p[i]) != tolower((unsigned char) t2->p[i])) {
			return (int) (unsigned char) t1->p[i] - (int) (unsigned char) t2->p[i];
		}
	}

	return 0;
}

int
txtcmp(const struct txt *t1, const struct txt *t2)
{
	size_t i;

	assert(t1 != NULL);
	assert(t2 != NULL);

	if (t1->n < t2->n) {
		return -1;
	}

	if (t1->n > t2->n) {
		return +1;
	}

	for (i = 0; i < t1->n; i++) {
		if (t1->p[i] != t2->p[i]) {
			return (int) (unsigned char) t1->p[i] - (int) (unsigned char) t2->p[i];
		}
	}

	return 0;
}

