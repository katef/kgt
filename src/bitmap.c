/*
 * Copyright 2008-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include "bitmap.h"

int
bm_get(const struct bm *bm, size_t i)
{
	assert(bm != NULL);
	assert(i <= UCHAR_MAX);

	return bm->map[i / CHAR_BIT] & (1 << i % CHAR_BIT);
}

void
bm_set(struct bm *bm, size_t i)
{
	assert(bm != NULL);
	assert(i <= UCHAR_MAX);

	bm->map[i / CHAR_BIT] |= (1 << i % CHAR_BIT);
}

void
bm_unset(struct bm *bm, size_t i)
{
	assert(bm != NULL);
	assert(i <= UCHAR_MAX);

	bm->map[i / CHAR_BIT] &= ~(1 << i % CHAR_BIT);
}

size_t
bm_next(const struct bm *bm, int i, int value)
{
	size_t n;

	assert(bm != NULL);
	assert(i / CHAR_BIT < UCHAR_MAX);

	/* this could be faster by incrementing per element instead of per bit */
	for (n = i + 1; n <= UCHAR_MAX; n++) {
		/* ...and this could be faster by using peter wegner's method */
		if (!(bm->map[n / CHAR_BIT] & (1 << n % CHAR_BIT)) == !value) {
			return n;
		}
	}

	return UCHAR_MAX + 1;
}

unsigned int
bm_count(const struct bm *bm)
{
	unsigned char c;
	unsigned int count;
	size_t n;

	assert(bm != NULL);

	count = 0;

	/* this could be faster using richard hamming's method */
	for (n = 0; n < sizeof bm->map; n++) {
		/* counting bits set for an element, peter wegner's method */
		for (c = bm->map[n]; c != 0; c &= c - 1) {
			count++;
		}
	}

	return count;
}

void
bm_clear(struct bm *bm)
{
	static const struct bm bm_empty;

	assert(bm != NULL);

	*bm = bm_empty;
}

void
bm_invert(struct bm *bm)
{
	size_t n;

	assert(bm != NULL);

	for (n = 0; n < sizeof bm->map; n++) {
		bm->map[n] = ~bm->map[n];
	}
}

