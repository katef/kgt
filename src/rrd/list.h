/* $Id$ */

#ifndef KGT_RRD_LIST_H
#define KGT_RRD_LIST_H

#include "../ast.h"

struct list {
	struct node *node;
	struct list *next;
};

void
list_push(struct list **list, struct node *node);

struct node *
list_pop(struct list **list);

void
list_free(struct list **list);

unsigned
list_count(const struct list *list);

#endif
