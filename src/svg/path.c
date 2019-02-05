/*
 * Copyright 2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdlib.h>

#include "../xalloc.h"

#include "path.h"

void
svg_path_h(struct path **paths, unsigned x, unsigned y, int n)
{
	struct path *new;

	assert(paths != NULL);
/* TODO: assert n != 0 */

	new = xmalloc(sizeof *new);

	new->type = PATH_H;
	new->x = x;
	new->y = y;
	new->u.n = n;

	new->next = *paths;
	*paths = new;
}

void
svg_path_v(struct path **paths, unsigned x, unsigned y, int n)
{
	struct path *new;

	assert(paths != NULL);
/* TODO: assert n != 0 */

	new = xmalloc(sizeof *new);

	new->type = PATH_V;
	new->x = x;
	new->y = y;
	new->u.n = n;

	new->next = *paths;
	*paths = new;
}

void
svg_path_move(struct path *n, unsigned *x, unsigned *y)
{
	assert(n != NULL);
	assert(x != NULL);
	assert(y != NULL);

	*x = n->x;
	*y = n->y;

	switch (n->type) {
	case PATH_H:
		*x += n->u.n;
		break;

	case PATH_V:
		*y += n->u.n;
		break;
	}
}

struct path *
svg_path_find_preceding(struct path *paths, const struct path *n)
{
	struct path *p;

	assert(n != NULL);

	/*
	 * Find any node that ends at the given position.
	 */

	for (p = paths; p != NULL; p = p->next) {
		unsigned nx, ny;

		/* self loop */
		if (n == p) {
			continue;
		}

		svg_path_move(p, &nx, &ny);

		if (nx == n->x && ny == n->y) {
			return p;
		}
	}

	return NULL;
}

struct path *
svg_path_find_following(struct path *paths, unsigned x, unsigned y)
{
	struct path *p;

	/*
	 * Find any node which starts from the given position.
	 */

	for (p = paths; p != NULL; p = p->next) {
		if (p->x == x && p->y == y) {
			return p;
		}
	}

	return NULL;
}

struct path *
svg_path_find_start(struct path *paths)
{
	struct path *p;

	/*
	 * Find any node which doesn't have a node connecting to it.
	 * That is, an entry point to a sequence.
	 */

	for (p = paths; p != NULL; p = p->next) {
		if (!svg_path_find_preceding(p->next, p)) {
			return p;
		}
	}

	/* then it's a loop; any will do */
	return paths;
}

static void
svg_path_free_one(struct path *n)
{
	assert(n != NULL);

	free(n);
}

void
svg_path_remove(struct path **paths, struct path *n)
{
	struct path **p;

	for (p = paths; *p; p = &(*p)->next) {
		if (*p == n) {
			struct path *next;

			next = (*p)->next;
			svg_path_free_one(*p);
			*p = next;
			break;
		}
	}
}

