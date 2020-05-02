/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>

#include "../txt.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

void
rrd_pretty_collapse(int *changed, struct node **n)
{
	assert(n != NULL);

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_LOOP:
		break;

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

	case NODE_ALT_SKIPPABLE:
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

