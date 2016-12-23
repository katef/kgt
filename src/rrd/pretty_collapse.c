/* $Id$ */

#include <stddef.h>

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static void
node_walk(struct node **n)
{
	switch ((*n)->type) {
		struct list **p;

	case NODE_ALT:
		if (list_count((*n)->u.alt) == 1) {
			struct node *dead;

			dead = *n;
			*n = (*n)->u.alt->node;
			dead->u.alt = NULL;
			node_free(dead);

			/* node changed */
			node_walk(n);

			return;
		}

		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(&(*p)->node);
		}

		return;

	case NODE_SEQ:
		if (list_count((*n)->u.seq) == 1) {
			struct node *dead;

			dead = *n;
			*n = (*n)->u.seq->node;
			dead->u.seq = NULL;
			node_free(dead);

			/* node changed */
			node_walk(n);

			return;
		}

		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(&(*p)->node);
		}

		return;

	case NODE_LOOP:
		node_walk(&(*n)->u.loop.forward);
		node_walk(&(*n)->u.loop.backward);

		return;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		return;
	}
}

void
rrd_pretty_collapse(struct node **rrd)
{
	node_walk(rrd);
}

