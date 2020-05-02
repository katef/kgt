/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "../txt.h"

#include "list.h"
#include "node.h"

#include "../xalloc.h"

void
node_free(struct node *n)
{
	struct list *p;

	if (n == NULL) {
		return;
	}

	switch (n->type) {
	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
		/* TODO: free (struct txt).p? */
		break;

	case NODE_RULE:
	case NODE_PROSE:
		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		for (p = n->u.alt; p != NULL; p = p->next) {
			node_free(p->node);
		}
		list_free(&n->u.alt);
		break;

	case NODE_SEQ:
		for (p = n->u.seq; p != NULL; p = p->next) {
			node_free(p->node);
		}
		list_free(&n->u.seq);
		break;

	case NODE_LOOP:
		node_free(n->u.loop.forward);
		node_free(n->u.loop.backward);
		break;
	}

	free(n);
}

struct node *
node_create_ci_literal(const struct txt *literal)
{
	struct node *new;

	assert(literal != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_CI_LITERAL;

	new->u.literal = *literal;

	return new;
}

struct node *
node_create_cs_literal(const struct txt *literal)
{
	struct node *new;

	assert(literal != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_CS_LITERAL;

	new->u.literal = *literal;

	return new;
}

struct node *
node_create_name(const char *name)
{
	struct node *new;

	assert(name != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_RULE;

	new->u.name = name;

	return new;
}

struct node *
node_create_prose(const char *prose)
{
	struct node *new;

	assert(prose != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_PROSE;

	new->u.prose = prose;

	return new;
}

struct node *
node_create_alt(struct list *alt)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_ALT;

	new->u.alt = alt;

	return new;
}

struct node *
node_create_alt_skippable(struct list *alt)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_ALT_SKIPPABLE;

	new->u.alt = alt;

	return new;
}

struct node *
node_create_seq(struct list *seq)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_SEQ;

	new->u.seq = seq;

	return new;
}

struct node *
node_create_loop(struct node *forward, struct node *backward)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_LOOP;

	new->u.loop.forward  = forward;
	new->u.loop.backward = backward;

	new->u.loop.min = 0;
	new->u.loop.max = 0;

	return new;
}

void
node_make_seq(struct node **n)
{
	struct list *new;

	assert(n != NULL);

	if (*n != NULL && (*n)->type == NODE_SEQ) {
		return;
	}

	new = NULL;

	list_push(&new, *n);

	*n = node_create_seq(new);
}

int
node_compare(const struct node *a, const struct node *b)
{
	if (a == NULL && b == NULL) {
		return 1;
	}

	if (a == NULL || b == NULL) {
		return 0;
	}

	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
	case NODE_CI_LITERAL:
		return 0 == txtcasecmp(&a->u.literal, &b->u.literal);

	case NODE_CS_LITERAL:
		return 0 == txtcmp(&a->u.literal, &b->u.literal);

	case NODE_RULE:
		return 0 == strcmp(a->u.name, b->u.name);

	case NODE_PROSE:
		return 0 == strcmp(a->u.prose, b->u.prose);

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		return list_compare(a->u.alt, b->u.alt);

	case NODE_SEQ:
		return list_compare(a->u.seq, b->u.seq);

	case NODE_LOOP:
		return node_compare(a->u.loop.forward,  b->u.loop.forward) &&
		       node_compare(a->u.loop.backward, b->u.loop.backward);
	}

	return 1;
}

void
loop_flip(struct node *n)
{
	struct node *tmp;

	assert(n != NULL);
	assert(n->type == NODE_LOOP);

	tmp = n->u.loop.backward;
	n->u.loop.backward = n->u.loop.forward;
	n->u.loop.forward  = tmp;
}

