/* $Id$ */

#ifndef KGT_RRD_BLIST_H
#define KGT_RRD_BLIST_H

/* a backwards list of nodes */
struct bnode {
	struct node *v;
	struct bnode *next;
};

void
b_push(struct bnode **list, struct node *v);

int
b_pop(struct bnode **list, struct node **out);

void
b_clear(struct bnode **list);

#endif
