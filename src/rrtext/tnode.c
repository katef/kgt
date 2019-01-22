/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#define _BSD_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../ast.h"
#include "../bitmap.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"

#include "io.h"
#include "tnode.h"

#include "../xalloc.h"

static struct tnode *
tnode_create_node(const struct node *node);

static struct tnode *
tnode_create_ellipsis(void);

/* made-up to suit text output */
static size_t
escputc(char *s, char c)
{
	assert(s != NULL);

	switch (c) {
	case '\\': return sprintf(s, "\\\\");
	case '\"': return sprintf(s, "\\\"");

	case '\a': return sprintf(s, "\\a");
	case '\b': return sprintf(s, "\\b");
	case '\f': return sprintf(s, "\\f");
	case '\n': return sprintf(s, "\\n");
	case '\r': return sprintf(s, "\\r");
	case '\t': return sprintf(s, "\\t");
	case '\v': return sprintf(s, "\\v");

	default:
		break;
	}

	if (!isprint((unsigned char) c)) {
		return sprintf(s, "\\x%02x", (unsigned char) c);
	}

	return sprintf(s, "%c", c);
}

static char *
esc_literal(const char *s)
{
	const char *p;
	char *a, *q;
	size_t n;

	n = strlen(s) * 4 + 1; /* worst case for \xXX */

	a = xmalloc(n);

	for (p = s, q = a; *p != '\0'; p++) {
		q += escputc(q, *p);
	}

	*q = '\0';

	return a;
}

static void
tnode_free_alt_list(struct tlist_alt *list)
{
	size_t i;

	assert(list != NULL);

	for (i = 0; i < list->n; i++) {
		tnode_free(list->a[i]);
	}

	free(list->a);
}

static void
tnode_free_seq_list(struct tlist_seq *list)
{
	size_t i;

	assert(list != NULL);

	for (i = 0; i < list->n; i++) {
		tnode_free(list->a[i]);
	}

	free(list->a);
}

void
tnode_free(struct tnode *n)
{
	if (n == NULL) {
		return;
	}

	switch (n->type) {
	case TNODE_SKIP:
	case TNODE_ELLIPSIS:
	case TNODE_RULE:
		break;

	case TNODE_LITERAL:
		free((void *) n->u.literal);
		break;

	case TNODE_ALT:
		tnode_free_alt_list(&n->u.alt);
		break;

	case TNODE_SEQ:
		tnode_free_seq_list(&n->u.seq);
		break;

	case TNODE_LOOP:
		tnode_free(n->u.loop.forward);
		tnode_free(n->u.loop.backward);
		break;
	}

	free(n);
}

static int
char_terminal(const struct node *node, unsigned char *c)
{
	assert(c != NULL);

	if (node == NULL) {
		return 0;
	}

	if (node->type != NODE_LITERAL) {
		return 0;
	}

	if (strlen(node->u.literal) != 1) {
		return 0;
	}

	*c = (unsigned) node->u.literal[0];

	return 1;
}

static void
collate_ranges(struct bm *bm, const struct list *list)
{
	const struct list *p;

	assert(bm != NULL);
	assert(list != NULL);

	bm_clear(bm);

	for (p = list; p != NULL; p = p->next) {
		unsigned char c;

		if (!char_terminal(p->node, &c)) {
			continue;
		}

		bm_set(bm, c);
	}
}

static const struct node *
find_node(const struct list *list, char d)
{
	const struct list *p;

	for (p = list; p != NULL; p = p->next) {
		unsigned char c;

		if (!char_terminal(p->node, &c)) {
			continue;
		}

		if (c == (unsigned char) d) {
			return p->node;
		}
	}

	assert(!"unreached");
}

static struct tlist_alt
tnode_create_alt_list(const struct list *list)
{
	const struct list *p;
	struct tlist_alt new;
	size_t i;
	struct bm bm;
	int hi, lo;

	new.n = list_count(list); /* worst case */
	if (new.n == 0) {
		new.a = NULL;
		return new;
	}

	collate_ranges(&bm, list);

	new.a = xmalloc(sizeof *new.a * new.n);

	hi = -1;

	for (i = 0, p = list; p != NULL; p = p->next) {
		unsigned char c;

		if (!char_terminal(p->node, &c)) {
			new.a[i++] = tnode_create_node(p->node);
			continue;
		}

		if (!bm_get(&bm, c)) {
			/* already output */
			continue;
		}

		/* start of range */
		lo = bm_next(&bm, hi, 1);
		if (lo > UCHAR_MAX) {
			/* end of list */
		}

		/* end of range */
		hi = bm_next(&bm, lo, 0);
		if (hi > UCHAR_MAX && lo < UCHAR_MAX) {
			hi = UCHAR_MAX;
		}

		if (!isalnum((unsigned char) lo) && isalnum((unsigned char) hi)) {
			new.a[i++] = tnode_create_node(find_node(list, lo));
			bm_unset(&bm, lo);

			hi = lo;
			continue;
		}

		/* bring down endpoint, if it's past the end of the class */
		if (isalnum((unsigned char) lo)) {
			size_t i;

			const struct {
				int (*is)(int);
				int end;
			} b[] = {
				{ isdigit, '9' },
				{ isupper, 'Z' },
				{ islower, 'z' }
			};

			/* XXX: assumes ASCII */
			for (i = 0; i < sizeof b / sizeof *b; i++) {
				if (b[i].is((unsigned char) lo)) {
					if (!b[i].is((unsigned char) hi)) {
						hi = b[i].end + 1;
					}
					break;
				}
			}

			assert(i < sizeof b / sizeof *b);
		}

		assert(hi > lo);

		switch (hi - lo) {
			int j;

		case 1:
		case 2:
		case 3:
			new.a[i++] = tnode_create_node(find_node(list, lo));
			bm_unset(&bm, lo);

			hi = lo;
			break;

		default:
			new.a[i++] = tnode_create_node(find_node(list, lo));
			new.a[i++] = tnode_create_ellipsis();
			new.a[i++] = tnode_create_node(find_node(list, hi - 1));

			for (j = lo; j <= hi - 1; j++) {
				bm_unset(&bm, j);
			}

			break;
		}
	}

	assert(i <= new.n);
	new.n = i;

	return new;
}

static struct tlist_seq
tnode_create_seq_list(const struct list *list)
{
	const struct list *p;
	struct tlist_seq new;
	size_t i;

	new.n = list_count(list);
	if (new.n == 0) {
		new.a = NULL;
		return new;
	}

	new.a = xmalloc(sizeof *new.a * new.n);

	for (i = 0, p = list; p != NULL; p = p->next) {
		new.a[i++] = tnode_create_node(p->node);
	}

	assert(i == new.n);

	return new;
}

static size_t
loop_label(unsigned min, unsigned max, char *s)
{
	assert(max >= min);
	assert(s != NULL);

	if (max == 1 && min == 1) {
		return sprintf(s, "(exactly once)");
	} else if (max == 0 && min > 0) {
		return sprintf(s, "(at least %u times)", min);
	} else if (max > 0 && min == 0) {
		return sprintf(s, "(up to %d times)", max);
	} else if (max > 0 && min == max) {
		return sprintf(s, "(%u times)", max);
	} else if (max > 1 && min > 1) {
		return sprintf(s, "(%u-%u times)", min, max);
	}

	*s = '\0';

	return 0;
}

static struct tnode *
tnode_create_ellipsis(void)
{
	struct tnode *new;

	new = xmalloc(sizeof *new);

	new->type = TNODE_ELLIPSIS;
	new->w = 1;
	new->y = 0;
	new->h = 1;

	return new;
}

static struct tnode *
tnode_create_node(const struct node *node)
{
	struct tnode *new;

	new = xmalloc(sizeof *new);

	if (node == NULL) {
		new->type = TNODE_SKIP;
		new->w = 0;
		new->y = 0;
		new->h = 1;

		return new;
	}

	switch (node->type) {
	case NODE_LITERAL:
		new->type = TNODE_LITERAL;
		new->u.literal = esc_literal(node->u.literal);
		new->w = strlen(new->u.literal) + 4;
		new->y = 0;
		new->h = 1;
		break;

	case NODE_RULE:
		new->type = TNODE_RULE;
		new->u.name = node->u.name;
		new->w = strlen(new->u.name) + 2;
		new->y = 0;
		new->h = 1;
		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		new->type = TNODE_ALT;

		/*
		 * TODO: decide whether to put the skip above or hang it below.
		 * It looks nicer below when the item being skipped is low in height,
		 * and where adjacent SEQ nodes do not themselves go above the line.
		 */

		if (node->type == NODE_ALT_SKIPPABLE) {
			struct list list;

			list.node = NULL;
			list.next = node->u.alt;

			new->u.alt = tnode_create_alt_list(&list);
		} else {
			new->u.alt = tnode_create_alt_list(node->u.alt);
		}

		{
			unsigned w;
			size_t i;

			w = 0;

			for (i = 0; i < new->u.alt.n; i++) {
				if (new->u.alt.a[i]->w > w) {
					w = new->u.alt.a[i]->w;
				}
			}

			new->w = w + 6;
		}

		{
			unsigned y;
			size_t i;

			i = 0;

			/*
			 * Alt lists hang below the line.
			 * The y-height of this node is the y-height of just the first list item
			 * because the first item is at the top of the list, plus the height of
			 * the skip node above that.
			 */

			y = 0;

			if (node->type == NODE_ALT_SKIPPABLE) {
				assert(new->u.alt.n > i);
				assert(new->u.alt.a[i] != NULL);
				assert(new->u.alt.a[i]->type == TNODE_SKIP);
				assert(new->u.alt.a[i]->h == 1);

				y += new->u.alt.a[i]->h + 1;
				i++;
			}

			if (new->u.alt.n > i) {
				assert(new->u.alt.a[i] != NULL);

				y += new->u.alt.a[i]->y;
			}

			new->y = y;
		}

		{
			unsigned h;
			size_t i;

			h = 0;

			for (i = 0; i < new->u.alt.n; i++) {
				h += 1 + new->u.alt.a[i]->h;
			}

			new->h = h - 1;
		}

		break;

	case NODE_SEQ:
		new->type = TNODE_SEQ;
		new->u.seq = tnode_create_seq_list(node->u.seq);

		{
			unsigned w;
			size_t i;

			w = 0;

			for (i = 0; i < new->u.seq.n; i++) {
				w += new->u.seq.a[i]->w + 2;
			}

			new->w = w - 2;
		}

		{
			unsigned y;
			size_t i;

			y = 0;

			for (i = 0; i < new->u.seq.n; i++) {
				if (new->u.seq.a[i]->y > y) {
					y = new->u.seq.a[i]->y;
				}
			}

			new->y = y;
		}

		{
			unsigned top = 0, bot = 1;
			size_t i;

			for (i = 0; i < new->u.seq.n; i++) {
				unsigned y, z;

				y = new->u.seq.a[i]->y;
				if (y > top) {
					top = y;
				}

				z = new->u.seq.a[i]->h;
				if (z - y > bot) {
					bot = z - y;
				}
			}

			new->h = bot + top;
		}

		break;

	case NODE_LOOP:
		new->type = TNODE_LOOP;
		new->u.loop.forward  = tnode_create_node(node->u.loop.forward);
		new->u.loop.backward = tnode_create_node(node->u.loop.backward);

		loop_label(node->u.loop.min, node->u.loop.max, new->u.loop.label);

		{
			unsigned w;
			unsigned wf, wb, cw;

			wf = new->u.loop.forward->w;
			wb = new->u.loop.backward->w;

			w = (wf > wb ? wf : wb) + 6;

			cw = strlen(new->u.loop.label);

			if (cw > 0) {
				if (cw + 6 > w) {
					w = cw + 6;
				}
			}

			new->w = w;
		}

		{
			new->y = new->u.loop.forward->y;
		}

		{
			unsigned h;

			h = new->u.loop.forward->h + new->u.loop.backward->h + 1;

			if (strlen(new->u.loop.label) > 0) {
				if (new->u.loop.backward->type != TNODE_SKIP) {
					h += 2;
				}
			}

			new->h = h;
		}

		break;
	}

	return new;
}

static void
tnode_set_rtl(struct tnode *n, int rtl)
{
	assert(n != NULL);

	n->rtl = rtl;

	switch (n->type) {
		size_t i;

	case TNODE_SKIP:
	case TNODE_ELLIPSIS:
	case TNODE_LITERAL:
	case TNODE_RULE:
		break;

	case TNODE_ALT:
		for (i = 0; i < n->u.alt.n; i++) {
			tnode_set_rtl(n->u.alt.a[i], rtl);
		}
		break;

	case TNODE_SEQ:
		for (i = 0; i < n->u.seq.n; i++) {
			tnode_set_rtl(n->u.seq.a[i], rtl);
		}
		break;

	case TNODE_LOOP:
		tnode_set_rtl(n->u.loop.forward,   rtl);
		tnode_set_rtl(n->u.loop.backward, !rtl);
		break;
	}
}

struct tnode *
rrd_to_tnode(const struct node *node)
{
	struct tnode *n;

	n = tnode_create_node(node);

	tnode_set_rtl(n, 0);

	return n;
}

