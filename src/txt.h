/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_TXT_H
#define KGT_TXT_H

struct txt {
	const char *p;
	size_t n;
};

int
txtcasecmp(const struct txt *t1, const struct txt *t2);

int
txtcmp(const struct txt *t1, const struct txt *t2);

#endif

