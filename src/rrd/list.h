/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_LIST_H
#define KGT_RRD_LIST_H

struct node;

struct list {
	struct node *node;
	struct list *next;
};

void
list_push(struct list **list, struct node *node);

struct node *
list_pop(struct list **list);

void
list_cat(struct list **dst, struct list *src);

int
list_compare(const struct list *a, const struct list *b);

struct list **
list_tail(struct list **head);

void
list_free(struct list **list);

unsigned
list_count(const struct list *list);

#endif
