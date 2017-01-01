/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include "../xalloc.h"

#include "list.h"

void
list_push(struct list **list, struct node *node)
{
	struct list *new;

	assert(list != NULL);
	assert(node != NULL);

	new = xmalloc(sizeof *new);
	new->node = node;
	new->next = *list;

	*list = new;
}

struct node *
list_pop(struct list **list)
{
	struct list *n;
	struct node *node;

	if (list == NULL || *list == NULL) {
		return NULL;
	}

	n = *list;
	*list = n->next;

	node = n->node;

	free(n);

	return node;
}

struct list **
list_tail(struct list **head)
{
	struct list **p;

	/* TODO: rewrite legibly */
	for (p = head; *p != NULL && (*p)->next != NULL; p = &(*p)->next)
		;

	if (*p == NULL) {
		return NULL;
	}

	return p;
}

void
list_free(struct list **list)
{
	while (*list != NULL) {
		(void) list_pop(list);
	}
}

unsigned
list_count(const struct list *list)
{
	const struct list *p;
	unsigned n;

	n = 0;

	for (p = list; p != NULL; p = p->next) {
		n++;
	}

	return n;
}

