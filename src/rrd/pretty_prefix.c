#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"
#include "node.h"

static int
process_loop(struct node *loop) {
	int suflen = 0;
	struct node *a, *b, *seq = NULL, **tail = &seq;

	if (loop->next == NULL || loop->u.loop.forward->type != NODE_SKIP) {
		return 0;
	}

	if (loop->u.loop.backward->type != NODE_SEQUENCE) {
		struct node *tmp;
		if (!node_compare(loop->u.loop.backward, loop->next)) {
			return 0;
		}

		tmp = loop->u.loop.forward;
		loop->u.loop.forward  = loop->u.loop.backward;
		loop->u.loop.backward = tmp;

		return 1;
	}

	for (a = loop->u.loop.backward->u.sequence, b = loop->next; a != NULL && b != NULL; a = a->next, b = b->next) {
		if (!node_compare(a, b)) {
			break;
		}
		*tail = a;
		tail = &a->next;
		suflen++;
	}

	if (suflen > 0) {
		loop->u.loop.backward = *tail;
		*tail = NULL;
		node_free(loop->u.loop.forward);
		loop->u.loop.forward = node_create_sequence(seq);
	}

	return suflen;
}

static struct node_walker pretty_collapse_prefixes;

static int
collapse_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	struct node *p;

	for (p = n->u.sequence; p != NULL; p = p->next) {
		int i, suffix_len;

		if (p->type != NODE_LOOP) {
			continue;
		}

		suffix_len = process_loop(p);

		for (i = 0; i < suffix_len; i++) {
			struct node *q = p->next;
			p->next = q->next;
			q->next = NULL;
			node_free(q);
		}
	}

	if (!node_walk_list(&n->u.sequence, &pretty_collapse_prefixes, depth + 1, arg)) {
		return 0;
	}

	node_collapse(np);

	return 1;
}

static struct node_walker pretty_collapse_prefixes = {
	NULL,
	NULL, NULL,
	NULL, collapse_sequence,
	NULL
};

void
rrd_pretty_prefixes(struct node **rrd)
{
	node_walk(rrd, &pretty_collapse_prefixes, 0, 0);
}
