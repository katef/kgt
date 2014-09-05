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
		case NODE_LIST:
			node_free(n->u.list.list);
			break;

		case NODE_LOOP:
			node_free(n->u.loop.forward);
			node_free(n->u.loop.backward);
			break;

		case NODE_SKIP:
		case NODE_LEAF:
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
node_create_leaf(enum leaf_type type, const char *text)
{
	struct node *new;

	assert(text != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_LEAF;
	new->next = NULL;

	new->u.leaf.type = type;
	new->u.leaf.text = text;

	return new;
}

struct node *
node_create_list(enum list_type type, struct node *list)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_LIST;
	new->next = NULL;

	new->u.list.type = type;
	new->u.list.list = list;

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

