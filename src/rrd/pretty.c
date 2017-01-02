/* $Id: pretty_suffix.c 185 2016-12-23 21:32:07Z kate $ */

/*
 * Railroad diagram beautification
 */

#include <stddef.h>
#include <assert.h>

#include "../xalloc.h"

#include "node.h"
#include "list.h"
#include "pretty.h"

static void
node_walk(void (*f)(int *, struct node **),
	int *changed, struct node **n)
{
	assert(n != NULL);
	assert(f != NULL);

	f(changed, n);

	switch ((*n)->type) {
		struct list **p;

	case NODE_ALT:
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk(f, changed, &(*p)->node);
		}
		break;

	case NODE_SEQ:
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk(f, changed, &(*p)->node);
		}
		break;

	case NODE_LOOP:
		node_walk(f, changed, &(*n)->u.loop.forward);
		node_walk(f, changed, &(*n)->u.loop.backward);
		break;

	case NODE_SKIP:
	case NODE_RULE:
	case NODE_LITERAL:
		break;
	}
}

void
rrd_pretty(struct node **rrd)
{
	int changed;
	int limit;
	size_t i;

	void (*f[])(int *, struct node **) = {
		rrd_pretty_collapse, rrd_pretty_redundant,
		rrd_pretty_collapse, rrd_pretty_roll,
		rrd_pretty_collapse, rrd_pretty_affixes,
		rrd_pretty_collapse, rrd_pretty_bottom,
		rrd_pretty_collapse
	};

	limit = 20;

	for (i = 0; i < sizeof f / sizeof *f; i++) {
		do {
			changed = 0;
			node_walk(f[i], &changed, rrd);
		} while (changed && !limit--);
	}
}

