/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * RRD node rewriting
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/rewrite.h"
#include "../rrd/node.h"
#include "../rrd/list.h"

static void
add_alt(struct list **list, const struct txt *t)
{
	struct node *node;
	struct txt q;

	assert(list != NULL);
	assert(t != NULL);
	assert(t->p != NULL);

	q = xtxtdup(t);

	node = node_create_cs_literal(&q);

	list_push(list, node);
}

/* TODO: centralise */
static void
f(struct list **list, const struct txt *t, char *p, size_t n)
{
	assert(list != NULL);
	assert(t != NULL);
	assert(t->p != NULL);

	if (n == 0) {
		add_alt(list, t);
		return;
	}

	if (!isalpha((unsigned char) *p)) {
		f(list, t, p + 1, n - 1);
		return;
	}

	*p = toupper((unsigned char) *p);
	f(list, t, p + 1, n - 1);
	*p = tolower((unsigned char) *p);
	f(list, t, p + 1, n - 1);
}

static void
rewrite_ci(struct node *n)
{
	struct txt tmp;
	size_t i;

	assert(n->type == NODE_CI_LITERAL);

	/* case is normalised during AST creation */
	for (i = 0; i < n->u.literal.n; i++) {
		if (!isalpha((unsigned char) n->u.literal.p[i])) {
			continue;
		}

		assert(islower((unsigned char) n->u.literal.p[i]));
	}

	assert(n->u.literal.p != NULL);
	tmp = n->u.literal;

	/* we repurpose the existing node, which breaks abstraction for freeing */
	n->type = NODE_ALT;
	n->u.alt = NULL;

	f(&n->u.alt, &tmp, (void *) tmp.p, tmp.n);

	free((void *) tmp.p);
}

static void
node_walk(struct node *n)
{
	if (n == NULL) {
		return;
	}

	switch (n->type) {
		const struct list *p;

	case NODE_CI_LITERAL:
		rewrite_ci(n);

		break;

	case NODE_CS_LITERAL:
	case NODE_RULE:

		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		for (p = n->u.alt; p != NULL; p = p->next) {
			node_walk(p->node);
		}

		break;

	case NODE_SEQ:
		for (p = n->u.seq; p != NULL; p = p->next) {
			node_walk(p->node);
		}

		break;

	case NODE_LOOP:
		node_walk(n->u.loop.forward);
		node_walk(n->u.loop.backward);

		break;
	}
}

void
rewrite_rrd_ci_literals(struct node *n)
{
	node_walk(n);
}

