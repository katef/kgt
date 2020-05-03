/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <stddef.h>

#include "../txt.h"
#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

/*
 * This transformation combines two equivalent nodes:
 *
 *  ||-- A B C -->------------------v--||
 *               |                  |
 *               ^-- C B A -- "|" --<
 *
 * by pushing one of them to the loop's .forward list:
 *
 *  ||---- A B -->-- C -------------v--||
 *               |                  |
 *               ^-- B A ---- "|" --<
 */
static void
roll_prefix(int *changed, struct list **entry, struct node *loop)
{
	struct list **p, **q;

	assert(entry != NULL);
	assert((*entry)->next != NULL);
	assert((*entry)->next->node == loop);
	assert(loop != NULL);
	assert(loop->type == NODE_LOOP);

	/* node before loop */
	p = entry;

	/* if loop .backward isn't a NODE_SEQ, make it one */
	node_make_seq(loop->invisible, &loop->u.loop.backward);

	assert(loop->u.loop.backward != NULL);
	assert(loop->u.loop.backward->type == NODE_SEQ);

	/* find node at tail of .backward */
	q = list_tail(&loop->u.loop.backward->u.seq);
	if (q == NULL) {
		return;
	}

	if (!node_compare((*p)->node, (*q)->node)) {
		return;
	}

	/* if loop .forward isn't a NODE_SEQ, make it one */
	node_make_seq(loop->invisible, &loop->u.loop.forward);

	assert(loop->u.loop.forward != NULL);
	assert(loop->u.loop.forward->type == NODE_SEQ);
	assert(loop->u.loop.forward->u.seq != NULL);

	/* if destination is a skip node, destroy that node first */
	/* TODO: centralise */
	if (loop->u.loop.forward->u.seq->node == NULL) {
		node_free(list_pop(&loop->u.loop.forward->u.seq));
	}

	/*
	 * Nominate one of the equivalent nodes (either would do),
	 * remove it from its current list, and prepend to .forward
	 * instead.
	 */
	/* TODO: centralise? list_transplant() */
	{
		assert((*q)->next == NULL);

		(*q)->next = loop->u.loop.forward->u.seq;
		loop->u.loop.forward->u.seq = (*q);

		*q = NULL;
	}

	/* we don't have empty lists */
	if (loop->u.loop.backward->u.seq == NULL) {
		node_free(loop->u.loop.backward);
		loop->u.loop.backward = NULL;
	}

	/* destroy the other node */
	node_free(list_pop(p));

	*changed = 1;
}

/*
 * This transformation combines two equivalent nodes:
 *
 *  ||-->------------------v-- C B A --||
 *      |                  |
 *      ^-- "|" -- A B C --<
 *
 * by pushing one of them to the loop's .backward list:
 *
 *  ||-->------------- C --v-- B A ----||
 *      |                  |
 *      ^-- "|" ---- A B --<
 */
static void
roll_suffix(int *changed, struct list **exit, struct node *loop)
{
	struct list **p, **q;

	assert(exit != NULL);
	assert(loop != NULL);
	assert(loop->type == NODE_LOOP);

	/* node after loop */
	p = exit;

	/* if loop .backward isn't a NODE_SEQ, make it one */
	node_make_seq(loop->invisible, &loop->u.loop.backward);

	assert(loop->u.loop.backward != NULL);
	assert(loop->u.loop.backward->type == NODE_SEQ);

	/* find node at head of .backward */
	q = &loop->u.loop.backward->u.seq;
	if (*q == NULL) {
		return;
	}

	if (!node_compare((*p)->node, (*q)->node)) {
		return;
	}

	/* if loop .forward isn't a NODE_SEQ, make it one */
	node_make_seq(loop->invisible, &loop->u.loop.forward);

	assert(loop->u.loop.forward != NULL);
	assert(loop->u.loop.forward->type == NODE_SEQ);
	assert(loop->u.loop.forward->u.seq != NULL);

	/* if destination is a skip node, destroy that node first */
	/* TODO: centralise */
	if (loop->u.loop.forward->u.seq->node == NULL) {
		node_free(list_pop(&loop->u.loop.forward->u.seq));
	}

	/*
	 * Nominate one of the equivalent nodes (either would do),
	 * remove it from its current list, and append to .forward
	 * instead.
	 */
	/* TODO: centralise? list_transplant() */
	{
		struct list *next;

		next = (*q)->next;
		(*q)->next = NULL;

		list_cat(&loop->u.loop.forward->u.seq, *q);

		*q = next;
	}

	/* we don't have empty lists */
	if (loop->u.loop.backward->u.seq == NULL) {
		node_free(loop->u.loop.backward);
		loop->u.loop.backward = NULL;
	}

	/* destroy the other node */
	node_free(list_pop(p));

	*changed = 1;
}

void
rrd_pretty_roll(int *changed, struct node **n)
{
	assert(n != NULL);

	if (*n == NULL) {
		return;
	}

	switch ((*n)->type) {
		struct list **p;

	case NODE_SEQ:
		/*
		 * Here we're looking one node ahead for the prefix to a loop,
		 * and passing the address of the subsquent loop node
		 * rather than searching from the head each time.
		 */
		for (p = &(*n)->u.seq; *p != NULL; p = &(*p)->next) {
			if ((*p)->next != NULL && ((*p)->next->node != NULL && (*p)->next->node->type == NODE_LOOP)) {
				roll_prefix(changed, p, (*p)->next->node);
				if (*changed) {
					break;
				}
			}
		}

		/*
		 * The suffix is the node immediately following a loop.
		 */
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			if ((*p)->node != NULL && (*p)->node->type == NODE_LOOP && (*p)->next != NULL) {
				roll_suffix(changed, &(*p)->next, (*p)->node);
				if (*changed) {
					break;
				}
			}
		}

		break;

	case NODE_CI_LITERAL:
	case NODE_CS_LITERAL:
	case NODE_RULE:
	case NODE_PROSE:
	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
	case NODE_LOOP:
		break;
	}
}

