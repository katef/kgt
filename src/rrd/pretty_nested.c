/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>

#include "../txt.h"
#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
nested_alt(int *changed, struct node *n)
{
	struct list **p;
	struct list **next;

	/* fold nested alts into this one */
	for (p = &n->u.alt; *p != NULL; p = next) {
		struct list **head, **tail;
		struct list *dead;

		next = &(*p)->next;

		if ((*p)->node == NULL || ((*p)->node->type != NODE_ALT && (*p)->node->type != NODE_ALT_SKIPPABLE)) {
			continue;
		}

		dead = *p;

		/* incoming inner list */
		head = &(*p)->node->u.alt;

		for (tail = head; *tail != NULL; tail = &(*tail)->next)
			;

		*tail = (*p)->next;
		(*p)->next = NULL;

		*p = *head;
		*head = NULL;

		next = p;

		node_free(dead->node);
		list_free(&dead);

		*changed = 1;
	}
}

static void
nested_seq(int *changed, struct node *n)
{
	struct list **p;
	struct list **next;

	/* fold nested seqs into this one */
	for (p = &n->u.alt; *p != NULL; p = next) {
		struct list **head, **tail;
		struct list *dead;

		next = &(*p)->next;

		if ((*p)->node == NULL || (*p)->node->type != NODE_SEQ) {
			continue;
		}

		dead = *p;

		/* incoming inner list */
		head = &(*p)->node->u.alt;

		for (tail = head; *tail != NULL; tail = &(*tail)->next)
			;

		*tail = (*p)->next;
		(*p)->next = NULL;

		*p = *head;
		*head = NULL;

		next = p;

		node_free(dead->node);
		list_free(&dead);

		*changed = 1;
	}
}

void
rrd_pretty_nested(int *changed, struct node **n)
{
	assert(n != NULL);

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		nested_alt(changed, *n);
		break;

	case NODE_SEQ:
		nested_seq(changed, *n);
		break;

	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_LOOP:
		break;
	}
}

