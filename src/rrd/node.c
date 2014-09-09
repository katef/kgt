/* $Id$ */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
		case NODE_RULE:
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
node_create_name(const char *name)
{
	struct node *new;

	assert(name != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_RULE;
	new->next = NULL;

	new->u.name = name;

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
		if (list->u.choice == NULL || list->u.choice->next != NULL) {
			return;
		}

		*n = list->u.choice;
		list->u.choice = NULL;

		break;

	case NODE_SEQUENCE:
		if (list->u.sequence == NULL || list->u.sequence->next != NULL) {
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

static int
node_compare_list(struct node *a, struct node *b, int once)
{
	struct node *pa = NULL, *pb = NULL;
	int result = 1;

	if (a->type != b->type) {
		return 0;
	}

	for (pa = a, pb = b; pa != NULL && pb != NULL; pa = pa->next, pb = pb->next) {
		switch (a->type) {
		case NODE_SKIP:
			break;
		case NODE_TERMINAL:
			result = result && 0 == strcmp(a->u.terminal, b->u.terminal);
			break;
		case NODE_RULE:
			result = result && 0 == strcmp(a->u.name, b->u.name);
			break;
		case NODE_CHOICE:
			result = result && node_compare_list(a->u.choice, b->u.choice, 0);
			break;
		case NODE_SEQUENCE:
			result = result && node_compare_list(a->u.sequence, b->u.sequence, 0);
			break;
		case NODE_LOOP:
			result = result &&
				node_compare_list(a->u.loop.forward, a->u.loop.forward, 0) &&
			    node_compare_list(a->u.loop.backward, a->u.loop.backward, 0);
			break;
		}
		if (once) {
			break;
		}
	}

	if (!once && (pa != NULL || pb != NULL)) {
		/* lists are of different length */
		return 0;
	}

	return result;
}

int node_compare(struct node *a, struct node *b) {
	return node_compare_list(a, b, 1);
}
