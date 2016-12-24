/* $Id$ */

#include <stddef.h>

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static void
node_walk(int *changed, struct node **n)
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
			node_walk(changed, n);

			break;
		}

		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(changed, &(*p)->node);
		}

		break;

	case NODE_SEQ:
		if (list_count((*n)->u.seq) == 1) {
			struct node *dead;

			dead = *n;
			*n = (*n)->u.seq->node;
			dead->u.seq = NULL;
			node_free(dead);

			*changed = 1;
			node_walk(changed, n);

			break;
		}

		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(changed, &(*p)->node);
		}

		break;

	case NODE_LOOP:
		node_walk(changed, &(*n)->u.loop.forward);
		node_walk(changed, &(*n)->u.loop.backward);

		break;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		break;
	}
}

void
rrd_pretty_collapse(int *changed, struct node **rrd)
{
	node_walk(changed, rrd);
}

