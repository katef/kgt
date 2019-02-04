/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_SVG_PATH_H
#define KGT_SVG_PATH_H

enum path_type {
	PATH_H, /* horizontal */
	PATH_V  /* vertical */
};

struct path {
	enum path_type type;

	unsigned x;
	unsigned y;

	union {
		int n;
	} u;

	struct path *next;
};

void
svg_path_h(struct path **paths, unsigned x, unsigned y, int n);

void
svg_path_v(struct path **paths, unsigned x, unsigned y, int n);

void
svg_path_free(struct path *path);

#endif
