/* $Id$ */

#include <stdlib.h>

#include "../xalloc.h"

#include "blist.h"

void
b_push(struct bnode **list, struct node *v)
{
	struct bnode bn;

	bn.v = v;
	bn.next = *list;

	*list = xmalloc(sizeof bn);
	**list = bn;
}

int
b_pop(struct bnode **list, struct node **out)
{
	struct bnode *n;

	if (list == NULL || *list == NULL) {
		return 0;
	}

	n = *list;
	*list = (**list).next;
	if (out) {
		*out = n->v;
	}

	free(n);

	return 1;
}

void
b_clear(struct bnode **list)
{
	while (*list) {
		(void) b_pop(list, NULL);
	}
}

