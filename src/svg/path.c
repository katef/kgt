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

	new = xmalloc(sizeof *new);

	new->type = PATH_V;
	new->x = x;
	new->y = y;
	new->u.n = n;

	new->next = *paths;
	*paths = new;
}

void
svg_path_free(struct path *paths)
{
	struct path *p, *next;

	for (p = paths; p->next != NULL; p = next) {
		next = p->next;

		free(p);
	}
}

