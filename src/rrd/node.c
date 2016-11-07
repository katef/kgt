/* $Id$ */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "rrd.h" /* XXX */
#include "node.h"

#include "../xalloc.h"

void
node_free(struct node *n)
{
	struct node *p, *next;

	if (n == NULL) {
		return;
	}

	for (p = n; p != NULL; p = next) {
		next = p->next;

		switch (n->type) {
		case NODE_SKIP:
		case NODE_LITERAL:
		case NODE_RULE:
			break;

		case NODE_ALT:
			node_free(n->u.alt);
			break;

		case NODE_SEQ:
			node_free(n->u.seq);
			break;

		case NODE_LOOP:
			node_free(n->u.loop.forward);
			node_free(n->u.loop.backward);
			break;
		}

		free(p);
	}
}

struct node *
node_create_skip(void)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_SKIP;
	new->next = NULL;

	return new;
}

struct node *
node_create_literal(const char *literal)
{
	struct node *new;

	assert(literal != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_LITERAL;
	new->next = NULL;

	new->u.literal = literal;

	return new;
}

struct node *
node_create_name(const char *name)
{
	struct node *new;

	assert(name != NULL);

	new = xmalloc(sizeof *new);

	new->type = NODE_RULE;
	new->next = NULL;

	new->u.name = name;

	return new;
}

struct node *
node_create_alt(struct node *alt)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_ALT;
	new->next = NULL;

	new->u.alt = alt;

	return new;
}

struct node *
node_create_seq(struct node *seq)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_SEQ;
	new->next = NULL;

	new->u.seq = seq;

	return new;
}

struct node *
node_create_loop(struct node *forward, struct node *backward)
{
	struct node *new;

	new = xmalloc(sizeof *new);

	new->type = NODE_LOOP;
	new->next = NULL;

	new->u.loop.forward  = forward;
	new->u.loop.backward = backward;

	new->u.loop.min = 0;
	new->u.loop.max = 0;

	return new;
}

void
node_collapse(struct node **n)
{
	struct node *list;

	list = *n;

	switch (list->type) {
	case NODE_ALT:
		/* TODO: list_count() */
		if (list->u.alt == NULL || list->u.alt->next != NULL) {
			return;
		}

		*n = list->u.alt;
		list->u.alt = NULL;

		break;

	case NODE_SEQ:
		if (list->u.seq == NULL || list->u.seq->next != NULL) {
			return;
		}

		*n = list->u.seq;
		list->u.seq = NULL;

		break;

	default:
		return;
	}

	node_free(list);

	node_collapse(n);
}

static int
node_compare_list(struct node *a, struct node *b, int once)
{
	struct node *pa = NULL, *pb = NULL;
	int result = 1;

	if (a->type != b->type) {
		return 0;
	}

	for (pa = a, pb = b; pa != NULL && pb != NULL; pa = pa->next, pb = pb->next) {
		switch (a->type) {
		case NODE_SKIP:
			break;

		case NODE_LITERAL:
			result = result && 0 == strcmp(a->u.literal, b->u.literal);
			break;

		case NODE_RULE:
			result = result && 0 == strcmp(a->u.name, b->u.name);
			break;

		case NODE_ALT:
			result = result && node_compare_list(a->u.alt, b->u.alt, 0);
			break;

		case NODE_SEQ:
			result = result && node_compare_list(a->u.seq, b->u.seq, 0);
			break;

		case NODE_LOOP:
			result = result &&
				node_compare_list(a->u.loop.forward,  a->u.loop.forward,  0) &&
			    node_compare_list(a->u.loop.backward, a->u.loop.backward, 0);
			break;
		}

		if (once) {
			break;
		}
	}

	if (!once && (pa != NULL || pb != NULL)) {
		/* lists are of different length */
		return 0;
	}

	return result;
}

int node_compare(struct node *a, struct node *b) {
	return node_compare_list(a, b, 1);
}

