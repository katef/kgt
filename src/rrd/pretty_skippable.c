/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>

#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
skippable_alt(int *changed, struct node *n)
{
	struct list **p, **next;
	struct list *dead;

	for (p = &n->u.alt; *p != NULL; p = next) {
		next = &(**p).next;

		if ((*p)->node == NULL) {
			n->type = NODE_ALT_SKIPPABLE;

			dead = *p;

			*p = *next;

			dead->next = NULL;
			node_free(dead->node);
			list_free(&dead);

			*changed = 1;
		}
	}

	/* TODO: if you're skippable and you contain nothing, have some other transformation remove it */
	/* TODO: ditto NULL in SEQs, and empty seqs, and empty loops */
}

void
rrd_pretty_skippable(int *changed, struct node **n)
{
	assert(n != NULL);

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
	case NODE_ALT:
		skippable_alt(changed, *n);
		break;
	}
}

