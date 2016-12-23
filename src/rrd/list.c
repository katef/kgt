/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include "list.h"

struct node *
list_pop(struct list **list)
{
	struct list *n;
	struct node *node;

	if (list == NULL || *list == NULL) {
		return NULL;
	}

	n = *list;
	*list = (**list).next;

	node = n->node;

	free(n);

	return node;
}

void
list_free(struct list **list)
{
	while (*list) {
		(void) list_pop(list);
	}
}

