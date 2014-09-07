/* $Id$ */

/*
 * Railroad diagram beautification
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../xalloc.h"

#include "rrd.h"
#include "blist.h"
#include "pretty.h"
#include "node.h"

static struct node_walker pretty_redundant;

static int
redundant_choice(struct node *n, struct node **np, int depth, void *arg)
{
	int nc = 0, isopt = 0;
	struct node **p, **loop = NULL;

	for (p = &n->u.choice; *p != NULL; p = &(**p).next) {
		nc++;

		if (!node_walk(p, &pretty_redundant, depth + 1, arg)) {
			return 0;
		}

		if ((**p).type == NODE_SKIP) {
			isopt = 1;
		}

		if ((**p).type == NODE_LOOP) {
			loop = p;
		}
	}

	if (nc == 2 && isopt && loop != NULL) {
		struct node *l;

		l = *loop;

		/* special case: if an optional loop has an empty half, we can elide the NODE_CHOICE */
		if (l->u.loop.forward->type == NODE_SKIP) {
			*np = *loop;
			*loop = NULL;
			node_free(n);
		} else if (l->u.loop.backward->type == NODE_SKIP) {
			struct node *tmp;

			tmp = l->u.loop.backward;
			l->u.loop.backward = l->u.loop.forward;
			l->u.loop.forward = tmp;
			*np = *loop;
			*loop = NULL;

			node_free(n);
		}
	} else {
		struct node **next;

		/* fold nested choices into this one */
		for (p = &n->u.choice; *p != NULL; p = next) {
			struct node **head, **tail;
			struct node *dead;

			next = &(*p)->next;

			if ((*p)->type != NODE_CHOICE) {
				continue;
			}

			dead = *p;

			/* incoming inner list */
			head = &(*p)->u.choice;

			for (tail = head; *tail != NULL; tail = &(*tail)->next)
				;

			*tail = (*p)->next;
			(*p)->next = NULL;

			*p = *head;
			*head = NULL;

			next = p;

			node_free(dead);
		}
	}

	return 1;
}

static int
redundant_loop(struct node *n, struct node **np, int depth, void *arg)
{
	struct node **inner = NULL;
	struct node *loop;

	if (!node_walk(&n->u.loop.forward, &pretty_redundant, depth + 1, arg)) {
		return 0;
	}

	if (!node_walk(&n->u.loop.backward, &pretty_redundant, depth + 1, arg)) {
		return 0;
	}

	if (n->u.loop.forward->type == NODE_LOOP && n->u.loop.backward->type == NODE_SKIP) {
		loop = n->u.loop.forward;
		if (loop->u.loop.forward->type == NODE_SKIP || loop->u.loop.backward->type == NODE_SKIP) {
			inner = &n->u.loop.forward;
		}
	} else if (n->u.loop.backward->type == NODE_LOOP && n->u.loop.forward->type == NODE_SKIP) {
		loop = n->u.loop.backward;
		if (loop->u.loop.forward->type == NODE_SKIP) {
			inner = &n->u.loop.backward;
		} else if (loop->u.loop.backward->type == NODE_SKIP) {
			struct node *tmp;
			tmp = loop->u.loop.backward;
			loop->u.loop.backward = loop->u.loop.forward;
			loop->u.loop.forward = tmp;
			inner = &n->u.loop.backward;
		}
	}

	if (inner) {
		*np = *inner;
		*inner = NULL;
		node_free(n);
	}

	return 1;
}

static struct node_walker pretty_redundant = {
	NULL,
	NULL, NULL,
	redundant_choice, NULL,
	redundant_loop
};

void
rrd_pretty_redundant(struct node **rrd)
{
	node_walk(rrd, &pretty_redundant, 0, NULL);
}

