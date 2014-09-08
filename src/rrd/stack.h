/* $Id$ */

#ifndef KGT_RRD_STACK_H
#define KGT_RRD_STACK_H

struct stack {
	struct node *node;
	struct stack *next;
};

void
stack_push(struct stack **stack, struct node *node);

struct node *
stack_pop(struct stack **stack);

void
stack_free(struct stack **stack);

#endif
