/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include "../xalloc.h"

#include "blist.h"

void
b_push(struct bnode **list, struct node *v)
{
	struct bnode bn;

	assert(v != NULL);

	bn.v = v;
	bn.next = *list;

	*list = xmalloc(sizeof bn);
	**list = bn;
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

