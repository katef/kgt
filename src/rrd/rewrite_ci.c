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

#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/rewrite.h"
#include "../rrd/node.h"
#include "../rrd/list.h"

static void
add_alt(struct list **list, const char *s)
{
	struct node *node;

	assert(list != NULL);
	assert(s != NULL);

	s = xstrdup(s);

	node = node_create_cs_literal(s);

	list_push(list, node);
}

/* TODO: centralise */
static void
f(struct list **list, char *s, char *p)
{
	assert(list != NULL);
	assert(s != NULL);

	if (*p == '\0') {
		add_alt(list, s);
		return;
	}

	if (!isalpha((unsigned char) *p)) {
		f(list, s, p + 1);
		return;
	}

	*p = toupper((unsigned char) *p);
	f(list, s, p + 1);
	*p = tolower((unsigned char) *p);
	f(list, s, p + 1);
}

static void
rewrite_ci(struct node *n)
{
	char *s, *p;

	assert(n->type == NODE_CI_LITERAL);

	s = (void *) n->u.literal;

	/* case is normalised during AST creation */
	for (p = s; *p != '\0'; p++) {
		if (!isalpha((unsigned char) *p)) {
			continue;
		}

		assert(islower((unsigned char) *p));
	}

	/* we repurpose the existing node, which breaks abstraction for freeing */
	n->type = NODE_ALT;
	n->u.alt = NULL;

	f(&n->u.alt, s, s);

	free(s);
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

