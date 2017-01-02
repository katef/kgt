/* $Id$ */

#include <assert.h>
#include <stddef.h>

#include "pretty.h"
#include "node.h"
#include "list.h"

void
rrd_pretty_collapse(int *changed, struct node **n)
{
	assert(n != NULL);
	assert(*n != NULL);

	switch ((*n)->type) {
	case NODE_ALT:
		if (list_count((*n)->u.alt) == 1) {
			struct node *dead;

			dead = *n;
			*n = (*n)->u.alt->node;
			dead->u.alt = NULL;
			node_free(dead);

			*changed = 1;
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
		}
		break;
	}
}

