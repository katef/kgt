/* $Id$ */

#ifndef KGT_RRD_LIST_H
#define KGT_RRD_LIST_H

#include "../ast.h"

struct list {
	struct node *node;
	struct list *next;
};

struct node *
list_pop(struct list **list);

void
list_free(struct list **list);

#endif
