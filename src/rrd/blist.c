/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include "../xalloc.h"

#include "blist.h"

void
b_push(struct bnode **list, struct node *v)
{
	struct bnode *new;

	assert(list != NULL);
	assert(v != NULL);

	new = xmalloc(sizeof *new);
	new->v = v;
	new->next = *list;

	*list = new;
}

struct node *
b_pop(struct bnode **list)
{
	struct bnode *n;
	struct node *v;

	if (list == NULL || *list == NULL) {
		return NULL;
	}

	n = *list;
	*list = (**list).next;

	v = n->v;

	free(n);

	return v;
}

void
b_clear(struct bnode **list)
{
	while (*list) {
		(void) b_pop(list);
	}
}

