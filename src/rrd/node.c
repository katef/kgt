/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include "rrd.h" /* XXX */
#include "node.h"

#include "../xalloc.h"

void
node_free(struct node *n)
{
	struct node *p, *next;

	if (n == NULL) {
		return;
	}

	for (p = n; p != NULL; p = next) {
		next = p->next;

		switch (n->type) {
		case NODE_SKIP:
		case NODE_TERMINAL:
		case NODE_IDENTIFIER:
			break;

		case NODE_CHOICE:
			node_free(n->u.choice);
			break;

		case NODE_SEQUENCE:
			node_free(n->u.sequence);
			break;

		case NODE_LOOP:
			node_free(n->u.loop.forward);
			node_free(n->u.loop.backward);
			break;
		}

		free(p);
	}
}

struct node *
node_create_skip(void)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_SKIP;
	new->next = NULL;

	return new;
}

struct node *
node_create_terminal(const char *terminal)
{
	struct node *new;

	assert(terminal != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_TERMINAL;
	new->next = NULL;

	new->u.terminal = terminal;

	return new;
}

struct node *
node_create_identifier(const char *identifier)
{
	struct node *new;

	assert(identifier != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_IDENTIFIER;
	new->next = NULL;

	new->u.identifier = identifier;

	return new;
}

struct node *
node_create_choice(struct node *choice)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_CHOICE;
	new->next = NULL;

	new->u.choice = choice;

	return new;
}

struct node *
node_create_sequence(struct node *sequence)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_SEQUENCE;
	new->next = NULL;

	new->u.sequence = sequence;

	return new;
}

struct node *
node_create_loop(struct node *forward, struct node *backward)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_LOOP;
	new->next = NULL;

	new->u.loop.forward  = forward;
	new->u.loop.backward = backward;

	return new;
}

void
node_collapse(struct node **n)
{
	struct node *list;

	list = *n;

	switch (list->type) {
	case NODE_CHOICE:
		/* TODO: list_count() */
		if (list->u.choice == NULL || list->u.choice->next == NULL) {
			return;
		}

		*n = list->u.choice;
		list->u.choice = NULL;

		break;

	case NODE_SEQUENCE:
		if (list->u.sequence == NULL || list->u.sequence->next == NULL) {
			return;
		}

		*n = list->u.sequence;
		list->u.sequence = NULL;

		break;

	default:
		return;
	}

	node_free(list);

	node_collapse(n);
}

