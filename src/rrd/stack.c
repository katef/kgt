#include <assert.h>
#include <stdlib.h>

#include "../xalloc.h"

#include "stack.h"

void
stack_push(struct stack **stack, struct node *node)
{
	struct stack *new;

	assert(stack != NULL);
	assert(node != NULL);

	new = xmalloc(sizeof *new);
	new->node = node;
	new->next = *stack;

	*stack = new;
}

struct node *
stack_pop(struct stack **stack)
{
	struct stack *n;
	struct node *node;

	if (stack == NULL || *stack == NULL) {
		return NULL;
	}

	n = *stack;
	*stack = (**stack).next;

	node = n->node;

	free(n);

	return node;
}

void
stack_free(struct stack **stack)
{
	while (*stack) {
		(void) stack_pop(stack);
	}
}

