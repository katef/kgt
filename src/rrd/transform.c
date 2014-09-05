/* $Id$ */

/*
 * AST to Railroad transformation
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../ast.h"
#include "../xalloc.h"

#include "rrd.h"

static const size_t node_size[] = {
	sizeof (struct node),
	sizeof (struct node_leaf),
	sizeof (struct node_list),
	sizeof (struct node_loop)
};

void *node_create(enum node_type type) {
	struct node *n;
	n = xmalloc(node_size[type]);
	n->next = 0;
	n->type = type;
	return n;
}

static void node_free_list(struct node *n) {
	struct node *p = n;
	while (p) {
		n = p->next;
		node_free(p);
		p = n;
	}
}

void node_free(struct node *n) {
	if (!n)
		return;
	switch (n->type) {
		case NT_LIST:
			node_free_list(((struct node_list *)n)->list);
			break;
		case NT_LOOP:
			node_free_list(((struct node_loop *)n)->forward);
			node_free_list(((struct node_loop *)n)->backward);
			break;
		default:
			break;
	}
	free(n);
}

void node_collapse(struct node **n) {
	struct node_list *list;
	if ((**n).type != NT_LIST)
		return;
	list = (struct node_list *)*n;
	if (list->list && !list->list->next) {
		*n = list->list;
		list->list = 0;
		node_free(&list->node);
		node_collapse(n);
	}
}

static int transform_alts(struct node **, struct ast_alt *);
static int transform_alt(struct node **, struct ast_alt *);
static int transform_term(struct node **, struct ast_term *);
static int transform_empty(struct node **);
static int transform_leaf(struct node **, struct ast_term *);
static int transform_group(struct node **, struct ast_alt *);

static int transform_alts(struct node **on, struct ast_alt *alts) {
	struct node *list = 0, **head = &list;
	struct ast_alt *p;

	for (p = alts; p; p = p->next) {
		if (!transform_alt(head, p))
			return 0;
		while (*head)
			head = &(**head).next;
	}

	*on = list;

	return 1;
}

static int transform_alt(struct node **on, struct ast_alt *alt) {
	struct node *list = 0, **head = &list;
	struct ast_term *p;

	for (p = alt->terms; p; p = p->next) {
		if (!transform_term(head, p))
			return 0;
		while (*head)
			head = &(**head).next;
	}

	if (!list->next) {
		*on = list;
	} else {
		struct node_list *sequence = node_create(NT_LIST);
		sequence->type = LIST_SEQUENCE;
		sequence->list = list;
		*on = &sequence->node;
	}

	return 1;
}

static int single_term(struct node **on, struct ast_term *term) {
	int ok;
	struct node *n;

	switch (term->type) {
		case TYPE_EMPTY:
			ok = transform_empty(&n);
			break;
		case TYPE_PRODUCTION:
		case TYPE_TERMINAL:
			ok = transform_leaf(&n, term);
			break;
		case TYPE_GROUP:
			ok = transform_group(&n, term->u.group);
			break;
		default:
			return 0;
	}

	if (!ok)
		return 0;

	*on = n;
	return 1;
}

static int optional_term(struct node **on, struct ast_term *term) {
	struct node *nothing;
	struct node_list *choice;

	nothing = node_create(NT_SKIP);

	choice = node_create(NT_LIST);
	choice->type = LIST_CHOICE;
	choice->list = nothing;

	*on = &choice->node;
	if (!single_term(&nothing->next, term))
		return 0;

	return 1;
}

static int oneormore_term(struct node **on, struct ast_term *term) {
	struct node *nothing;
	struct node_list *choice;
	struct node_loop *loop;

	nothing = node_create(NT_SKIP);
	choice = node_create(NT_LIST);
	choice->type = LIST_CHOICE;

	loop = node_create(NT_LOOP);
	loop->forward = &choice->node;
	loop->backward = nothing;

	*on = &loop->node;
	if (!single_term(&choice->list, term))
		return 0;

	node_collapse(&loop->forward);
	node_collapse(&loop->backward);

	return 1;
}

static int zeroormore_term(struct node **on, struct ast_term *term) {
	struct node *nothing;
	struct node_list *choice;
	struct node_loop *loop;

	nothing = node_create(NT_SKIP);
	choice = node_create(NT_LIST);
	choice->type = LIST_CHOICE;

	loop = node_create(NT_LOOP);
	loop->forward = nothing;
	loop->backward = &choice->node;

	*on = &loop->node;
	if (!single_term(&nothing->next, term))
		return 0;

	node_collapse(&loop->forward);
	node_collapse(&loop->backward);

	return 1;
}

static int transform_term(struct node **on, struct ast_term *term) {
	size_t i;

	struct {
		unsigned int min;
		unsigned int max;
		int (*f)(struct node **on, struct ast_term *term);
	} a[] = {
		{ 1, 1, single_term     },
		{ 0, 1, optional_term   },
		{ 1, 0, oneormore_term  },
		{ 0, 0, zeroormore_term }
	};

	/* TODO: our rrd tree can't express finite term repetition */
	assert(term->max <= 1);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (term->min == a[i].min && term->min == a[i].min) {
			return a[i].f(on, term);
		}
	}

	return 0;
}

static int transform_empty(struct node **on) {
	struct node *n = node_create(NT_SKIP);
	*on = n;
	return 1;
}

static int transform_leaf(struct node **on, struct ast_term *term) {
	struct node_leaf *n = node_create(NT_LEAF);
	if (term->type == TYPE_PRODUCTION) {
		n->type = LEAF_IDENTIFIER;
		n->text = term->u.name;
	} else if (term->type == TYPE_TERMINAL) {
		n->type = LEAF_TERMINAL;
		n->text = term->u.literal;
	}
	*on = &n->node;
	return 1;
}

static int transform_group(struct node **on, struct ast_alt *group) {
	struct node_list *list = node_create(NT_LIST);
	list->type = LIST_CHOICE;
	*on = &list->node;
	if (!transform_alts(&list->list, group))
		return 0;
	node_collapse(on);
	return 1;
}

int ast_to_rrd(struct ast_production *ast, struct node **rrd) {
	struct node_list *choice = node_create(NT_LIST);
	choice->type = LIST_CHOICE;
	if (!transform_alts(&choice->list, ast->alts))
		return 0;
	*rrd = &choice->node;
	node_collapse(rrd);
	return 1;
}

static int node_call_walker(struct node **n, const struct node_walker *ws, int depth, void *a) {
	if ((**n).type == NT_SKIP) {
		return ws->visit_nothing ? ws->visit_nothing(*n, n, depth, a) : -1;
	}
	if ((**n).type == NT_LEAF) {
		struct node_leaf *leaf = (struct node_leaf *)*n;
		if (leaf->type == LEAF_IDENTIFIER)
			return ws->visit_identifier ? ws->visit_identifier(leaf, n, depth, a) : -1;
		else
			return ws->visit_terminal ? ws->visit_terminal(leaf, n, depth, a) : -1;
	}
	if ((**n).type == NT_LIST) {
		struct node_list *list = (struct node_list *)*n;
		if (list->type == LIST_CHOICE)
			return ws->visit_choice ? ws->visit_choice(list, n, depth, a) : -1;
		else
			return ws->visit_sequence ? ws->visit_sequence(list, n, depth, a) : -1;
	}
	if ((**n).type == NT_LOOP) {
		struct node_loop *loop = (struct node_loop *)*n;
		return ws->visit_loop ? ws->visit_loop(loop, n, depth, a) : -1;
	}
	return -1;
}

int node_walk_list(struct node **n, const struct node_walker *ws, int depth, void *a) {
	for (; *n; n = &(**n).next) {
		if (!node_walk(n, ws, depth, a))
			return 0;
	}
	return 1;
}

int node_walk(struct node **n, const struct node_walker *ws, int depth, void *a) {
	int r = node_call_walker(n, ws, depth, a);
	if (r == 0) {
		return 0;
	} else if (r == -1) {
		if ((**n).type == NT_LIST) {
			struct node_list *list = (struct node_list *)*n;
			if (!node_walk_list(&list->list, ws, depth + 1, a))
				return 0;
		} else if ((**n).type == NT_LOOP) {
			struct node_loop *loop = (struct node_loop *)*n;
			if (!node_walk(&loop->forward, ws, depth + 1, a))
				return 0;
			if (!node_walk(&loop->backward, ws, depth + 1, a))
				return 0;
		} else {
			r = 1;
		}
	}
	return 1;
}
