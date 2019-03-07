/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stdlib.h>

#include "../txt.h"
#include "../xalloc.h"

#include "list.h"
#include "node.h"

void
list_push(struct list **list, struct node *node)
{
	struct list *new;

	assert(list != NULL);

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

void
list_cat(struct list **dst, struct list *src)
{
	struct list **p;

	for (p = dst; *p != NULL; p = &(*p)->next)
		;

	src->next = *p;
	*p = src;
}

int
list_compare(const struct list *a, const struct list *b)
{
	const struct list *p, *q;

	for (p = a, q = b; p != NULL && q != NULL; p = p->next, q = q->next) {
		if (!node_compare(p->node, q->node)) {
			return 0;
		}
	}

	if (p != NULL || q != NULL) {
		/* lists are of different length */
		return 0;
	}

	return 1;
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

