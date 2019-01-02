/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#define _BSD_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"
#include "../rrd/stack.h"

#include "io.h"
#include "tnode.h"

#include "../xalloc.h"

static struct tnode *
tnode_create_node(const struct node *node);

/* XXX */
static unsigned node_walk_dim_w(const struct node *n);
static unsigned node_walk_dim_y(const struct node *n);
static unsigned node_walk_dim_h(const struct node *n);

static void
tnode_free_tlist(struct tlist *list)
{
	size_t i;

	assert(list != NULL);

	for (i = 0; i < list->n; i++) {
		tnode_free(list->a[i]);
	}

	free(list->a);
}

void
tnode_free(struct tnode *n)
{
	if (n == NULL) {
		return;
	}

	switch (n->type) {
	case TNODE_LITERAL:
	case TNODE_RULE:
		break;

	case TNODE_ALT:
	case TNODE_ALT_SKIPPABLE:
		tnode_free_tlist(&n->u.alt);
		break;

	case TNODE_SEQ:
		tnode_free_tlist(&n->u.seq);
		break;

	case TNODE_LOOP:
		tnode_free(n->u.loop.forward);
		tnode_free(n->u.loop.backward);
		break;
	}

	free(n);
}

static struct tlist
tnode_create_list(const struct list *list)
{
	const struct list *p;
	struct tlist new;
	size_t i;

	new.n = list_count(list);
	if (new.n == 0) {
		new.a = NULL;
		return new;
	}

	new.a = xmalloc(sizeof *new.a * new.n);

	for (i = 0, p = list; i < new.n; i++, p = p->next) {
		assert(p != NULL);

		new.a[i] = tnode_create_node(p->node);
	}

	return new;
}

static enum tnode_looptype
tnode_looptype(const struct node *loop)
{
	if (loop->u.loop.max == 1 && loop->u.loop.min == 1) {
		return TNODE_LOOP_ONCE;
	} else if (loop->u.loop.max == 0 && loop->u.loop.min > 0) {
		return TNODE_LOOP_ATLEAST;
	} else if (loop->u.loop.max > 0 && loop->u.loop.min == 0) {
		return TNODE_LOOP_UPTO;
	} else if (loop->u.loop.max > 0 && loop->u.loop.min == loop->u.loop.max) {
		return TNODE_LOOP_EXACTLY;
	} else if (loop->u.loop.max > 1 && loop->u.loop.min > 1) {
		return TNODE_LOOP_BETWEEN;
	}

	assert(!"unreached");
	return 0;
}

static struct tnode *
tnode_create_node(const struct node *node)
{
	struct tnode *new;

	if (node == NULL) {
		return NULL;
	}

	new = xmalloc(sizeof *new);

	new->w = node_walk_dim_w(node);
	new->y = node_walk_dim_y(node);
	new->h = node_walk_dim_h(node);

	switch (node->type) {
	case NODE_LITERAL:
		new->type = TNODE_LITERAL;
		new->u.literal = node->u.literal;
		break;

	case NODE_RULE:
		new->type = TNODE_RULE;
		new->u.name = node->u.name;
		break;

	case NODE_ALT:
		new->type = TNODE_ALT;
		new->u.alt = tnode_create_list(node->u.alt);
		break;

	case NODE_ALT_SKIPPABLE:
		new->type = TNODE_ALT_SKIPPABLE;
		new->u.alt = tnode_create_list(node->u.alt);
		break;

	case NODE_SEQ:
		new->type = TNODE_SEQ;
		new->u.seq = tnode_create_list(node->u.seq);
		break;

	case NODE_LOOP:
		new->type = TNODE_LOOP;
		new->u.loop.looptype = tnode_looptype(node);
		new->u.loop.forward  = tnode_create_node(node->u.loop.forward);
		new->u.loop.backward = tnode_create_node(node->u.loop.backward);
		new->u.loop.min = node->u.loop.min;
		new->u.loop.max = node->u.loop.max;
		break;
	}

	return new;
}

struct tnode *
rrd_to_tnode(const struct node *node)
{
	return tnode_create_node(node);
}

static unsigned
node_walk_dim_w(const struct node *n)
{
	if (n == NULL) {
		return 0;
	}

	switch (n->type) {
		const struct list *p;
		unsigned w;

	case NODE_LITERAL:
		return strlen(n->u.literal) + 4;

	case NODE_RULE:
		return strlen(n->u.name) + 2;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		w = 0;

		for (p = n->u.alt; p != NULL; p = p->next) {
			unsigned wn;

			wn = node_walk_dim_w(p->node);
			if (wn > w) {
				w = wn;
			}
		}

		return w + 6;

	case NODE_SEQ:
		w = 0;

		for (p = n->u.seq; p != NULL; p = p->next) {
			w += node_walk_dim_w(p->node) + 2;
		}

		return w - 2;

	case NODE_LOOP:
		{
			unsigned wf, wb, cw;

			wf = node_walk_dim_w(n->u.loop.forward);
			wb = node_walk_dim_w(n->u.loop.backward);

			w = (wf > wb ? wf : wb) + 6;

			cw = loop_label(n, NULL);

			if (cw > 0) {
				if (cw + 6 > w) {
					w = cw + 6;
				}
			}
		}

		return w;
	}
}

static unsigned
node_walk_dim_y(const struct node *n)
{
	if (n == NULL) {
		return 0;
	}

	switch (n->type) {
		const struct list *p;
		unsigned y;

	case NODE_LITERAL:
		return 0;

	case NODE_RULE:
		return 0;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		assert(n->u.alt != NULL);

		p = n->u.alt;

		/*
		 * Alt lists hang below the line.
		 * The y-height of this node is the y-height of just the first list item
		 * because the first item is at the top of the list, plus the height of
		 * the skip node above that.
 		 */
		y = node_walk_dim_y(p->node);

		if (n->type == NODE_ALT_SKIPPABLE) {
			y += 2;
		}

		return y;

	case NODE_SEQ:
		y = 0;

		for (p = n->u.seq; p != NULL; p = p->next) {
			unsigned z;

			z = node_walk_dim_y(p->node);
			if (z > y) {
				y = z;
			}
		}

		return y;

	case NODE_LOOP:
		return node_walk_dim_y(n->u.loop.forward);
	}
}

static unsigned
node_walk_dim_h(const struct node *n)
{
	if (n == NULL) {
		return 1;
	}

	switch (n->type) {
		const struct list *p;
		unsigned h;

	case NODE_LITERAL:
		return 1;

	case NODE_RULE:
		return 1;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		h = 0;

		assert(n->u.alt != NULL);

		for (p = n->u.alt; p != NULL; p = p->next) {
			h += 1 + node_walk_dim_h(p->node);
		}

		if (n->type == NODE_ALT_SKIPPABLE) {
 			h += 2;
		}

		return h - 1;

	case NODE_SEQ:
		{
			unsigned top = 0, bot = 1;

			assert(n->u.seq != NULL);

			for (p = n->u.seq; p != NULL; p = p->next) {
				unsigned y, z;

				y = node_walk_dim_y(p->node);
				if (y > top) {
					top = y;
				}

				z = node_walk_dim_h(p->node);
				if (z - y > bot) {
					bot = z - y;
				}
			}

			return bot + top;
		}

		break;

	case NODE_LOOP:
		h = node_walk_dim_h(n->u.loop.forward) + node_walk_dim_h(n->u.loop.backward) + 1;

		if (loop_label(n, NULL) > 0) {
			if (n->u.loop.backward != NULL) {
				h += 2;
			}
		}

		return h;
	}
}

