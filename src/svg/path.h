/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_SVG_PATH_H
#define KGT_SVG_PATH_H

enum path_type {
	PATH_H, /* horizontal */
	PATH_V, /* vertical */
	PATH_Q  /* quadratic */
};

struct path {
	enum path_type type;

	unsigned x;
	unsigned y;

	union {
		int n;
		int q[4];
	} u;

	struct path *next;
};

void
svg_path_h(struct path **paths, unsigned x, unsigned y, int n);

void
svg_path_v(struct path **paths, unsigned x, unsigned y, int n);

void
svg_path_q(struct path **paths, unsigned x, unsigned y, int rx, int ry, int mx, int my);

void
svg_path_move(struct path *n, unsigned *x, unsigned *y);

struct path *
svg_path_find_preceding(struct path *paths, const struct path *n);

struct path *
svg_path_find_following(struct path *paths, unsigned x, unsigned y);

struct path *
svg_path_find_start(struct path *paths);

void
svg_path_remove(struct path **paths, struct path *n);

void
svg_path_consolidate(struct path **paths);

#endif
