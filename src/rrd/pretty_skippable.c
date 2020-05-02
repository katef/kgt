/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "../txt.h"
#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
skippable_alt(int *changed, struct node *n)
{
	struct list *p;

	for (p = n->u.alt; p != NULL; p = p->next) {
		if (p->node == NULL) {
			n->type = NODE_ALT_SKIPPABLE;
			*changed = 1;
		}
	}

	/* TODO: if you're skippable and you contain nothing, have some other transformation remove it */
}

static void
redundant_skip(int *changed, struct list **list)
{
	struct list **p;
	struct list **next;

	/*
	 * If there are skip nodes (NULL) in a seq or skippable alt,
	 * just remove them - they have no semantic effect.
	 */

	for (p = list; *p != NULL; p = next) {
		next = &(*p)->next;

		if ((*p)->node == NULL) {
			struct list *dead;

			dead = *p;
			*p = (*p)->next;

			dead->next = NULL;
			list_free(&dead);

			*changed = 1;

			next = p;
		}
	}
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

	case NODE_ALT_SKIPPABLE:
		redundant_skip(changed, &(*n)->u.alt);
		break;

	case NODE_SEQ:
		redundant_skip(changed, &(*n)->u.seq);
		break;

	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_LOOP:
		break;
	}
}

