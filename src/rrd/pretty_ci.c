/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../txt.h"
#include "../xalloc.h"

#include "pretty.h"
#include "node.h"
#include "list.h"

static void
ci_alt(int *changed, struct node *n)
{
	struct list *list;
	struct list *p;

	/*
	 * If every text node in an alt list is a single character,
	 * convert them to case-sensitive expansions. Then the ellipsis
	 * replacements happens per usual during tnode rewriting.
	 * The end effect is to produce an ellipsis for uppercase
	 * and an ellipsis for lowercase.
	 */

	list = NULL;

	for (p = n->u.alt; p != NULL; p = p->next) {
		if (p->node == NULL) {
			continue;
		}

		switch (p->node->type) {
		case NODE_CI_LITERAL:
		case NODE_CS_LITERAL:
			if (p->node->u.literal.n != 1) {
				return;
			}

			break;

		case NODE_RULE:
		case NODE_PROSE:
		case NODE_ALT:
		case NODE_ALT_SKIPPABLE:
		case NODE_SEQ:
		case NODE_LOOP:
			break;
		}
	}

	for (p = n->u.alt; p != NULL; p = p->next) {
		if (p->node == NULL) {
			continue;
		}

		switch (p->node->type) {
			struct node *new;
			struct txt t;

		case NODE_CI_LITERAL:
			if (!isalpha((unsigned char) *p->node->u.literal.p)) {
				break;
			}

			p->node->type = NODE_CS_LITERAL;
			* (char *) p->node->u.literal.p = tolower((unsigned char) *p->node->u.literal.p);

			t = xtxtdup(&p->node->u.literal);
			* (char *) t.p = toupper((unsigned char) *t.p);
			new = node_create_cs_literal(p->node->invisible, &t);
			list_push(&list, new);

			*changed = 1;

			break;

		case NODE_CS_LITERAL:
		case NODE_RULE:
		case NODE_PROSE:
		case NODE_ALT:
		case NODE_ALT_SKIPPABLE:
		case NODE_SEQ:
		case NODE_LOOP:
			break;
		}
	}

	/* append list */
	{
		struct list **tail;

		/* TODO: centralise with list_tail() */
		for (tail = &n->u.alt; *tail != NULL; tail = &(*tail)->next)
			;

		assert(*tail == NULL);
		*tail = list;
	}
}

void
rrd_pretty_ci(int *changed, struct node **n)
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
	case NODE_SEQ:
	case NODE_LOOP:
		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		ci_alt(changed, *n);
		break;
	}
}

