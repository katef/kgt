/* $Id$ */

#include <stddef.h>

#include "rrd.h"
#include "pretty.h"
#include "node.h"
#include "list.h"

static void
node_walk(struct node **n)
{
	struct node *node;

	node = *n;

	switch (node->type) {
	case NODE_ALT:
		/* TODO: list_count() */
		if (node->u.alt == NULL || node->u.alt->next != NULL) {
			return;
		}

		*n = node->u.alt->node;
		node->u.alt = NULL;

		break;

	case NODE_SEQ:
		if (node->u.seq == NULL || node->u.seq->next != NULL) {
			return;
		}

		*n = node->u.seq->node;
		node->u.seq = NULL;

		break;

	default:
		return;
	}

	node_free(node);

	node_walk(n);
}

void
rrd_pretty_collapse(struct node **rrd)
{
	node_walk(rrd);
}

