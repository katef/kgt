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

#include "tnode.h"

#include "../xalloc.h"

static struct tnode *
tnode_create_node(const struct node *node, int rtl, const struct dim *dim);

static struct tnode *
tnode_create_ellipsis(const struct dim *dim);

static void
swap(struct tnode **a, struct tnode **b)
{
	struct tnode *tmp;

	assert(a != NULL);
	assert(b != NULL);

	tmp = *a;
	*a = *b;
	*b = tmp;
}

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
tnode_free_vlist(struct tnode_vlist *list)
{
	size_t i;

	assert(list != NULL);

	for (i = 0; i < list->n; i++) {
		tnode_free(list->a[i]);
	}

	free(list->a);
	free(list->b);
}

static void
tnode_free_hlist(struct tnode_hlist *list)
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
	case TNODE_ARROW:
	case TNODE_ELLIPSIS:
	case TNODE_RULE:
		break;

	case TNODE_CI_LITERAL:
	case TNODE_CS_LITERAL:
		free((void *) n->u.literal);
		break;

	case TNODE_PROSE:
		free((void *) n->u.prose);
		break;

	case TNODE_VLIST:
		tnode_free_vlist(&n->u.vlist);
		break;

	case TNODE_HLIST:
		tnode_free_hlist(&n->u.hlist);
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

	/* we collate ranges for case-sensitive strings only */
	if (node->type != NODE_CS_LITERAL) {
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

static struct tnode_vlist
tnode_create_alt_list(const struct list *list, int rtl, const struct dim *dim)
{
	const struct list *p;
	struct tnode_vlist new;
	size_t i;
	struct bm bm;
	int hi, lo;

	assert(dim != NULL);

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
			new.a[i++] = tnode_create_node(p->node, rtl, dim);
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
			new.a[i++] = tnode_create_node(find_node(list, lo), rtl, dim);
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
			new.a[i++] = tnode_create_node(find_node(list, lo), rtl, dim);
			bm_unset(&bm, lo);

			hi = lo;
			break;

		default:
			new.a[i++] = tnode_create_node(find_node(list, lo), rtl, dim);
			new.a[i++] = tnode_create_ellipsis(dim);
			new.a[i++] = tnode_create_node(find_node(list, hi - 1), rtl, dim);

			for (j = lo; j <= hi - 1; j++) {
				bm_unset(&bm, j);
			}

			break;
		}
	}

	assert(i <= new.n);
	new.n = i;

	new.b = xmalloc(sizeof *new.b * new.n);

	return new;
}

static struct tnode_hlist
tnode_create_hlist(const struct list *list, int rtl, const struct dim *dim)
{
	const struct list *p;
	struct tnode_hlist new;
	size_t i;

	assert(dim != NULL);

	new.n = list_count(list);
	if (new.n == 0) {
		new.a = NULL;
		return new;
	}

	new.a = xmalloc(sizeof *new.a * new.n);

	for (i = 0, p = list; p != NULL; p = p->next) {
		new.a[i++] = tnode_create_node(p->node, rtl, dim);
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
tnode_create_ellipsis(const struct dim *dim)
{
	struct tnode *new;

	assert(dim != NULL);

	new = xmalloc(sizeof *new);

	new->type = TNODE_ELLIPSIS;
	new->w = 1;
	new->a = 0;
	new->d = dim->ellipsis_depth;

	return new;
}

static struct tnode *
tnode_create_node(const struct node *node, int rtl, const struct dim *dim)
{
	struct tnode *new;

	assert(dim != NULL);

	new = xmalloc(sizeof *new);

	new->rtl = rtl;

	if (node == NULL) {
		new->type = TNODE_SKIP;
		new->w = 0;
		new->a = 0;
		new->d = 1;

		return new;
	}

	switch (node->type) {
	case NODE_CI_LITERAL:
		new->type = TNODE_CI_LITERAL;
		new->u.literal = esc_literal(node->u.literal);
		dim->literal_string(new->u.literal, &new->w, &new->a, &new->d);
		new->w += dim->literal_padding + dim->ci_marker;
		break;

	case NODE_CS_LITERAL:
		new->type = TNODE_CS_LITERAL;
		new->u.literal = esc_literal(node->u.literal);
		dim->literal_string(new->u.literal, &new->w, &new->a, &new->d);
		new->w += dim->literal_padding;
		break;

	case NODE_RULE:
		new->type = TNODE_RULE;
		new->u.name = node->u.name;
		dim->rule_string(new->u.name, &new->w, &new->a, &new->d);
		new->w += dim->rule_padding;
		break;

	case NODE_PROSE:
		new->type = TNODE_PROSE;
		new->u.prose = node->u.prose;
		dim->rule_string(new->u.prose, &new->w, &new->a, &new->d);
		if (!unquoted_prose(node->u.prose)) {
			new->w += dim->prose_padding;
		}
		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		new->type = TNODE_VLIST;

		/*
		 * TODO: decide whether to put the skip above or hang it below.
		 * It looks nicer below when the item being skipped is low in height,
		 * and where adjacent SEQ nodes do not themselves go above the line.
		 */

		if (node->type == NODE_ALT_SKIPPABLE) {
			struct list list;

			list.node = NULL;
			list.next = node->u.alt;

			new->u.vlist = tnode_create_alt_list(&list, rtl, dim);
		} else {
			new->u.vlist = tnode_create_alt_list(node->u.alt, rtl, dim);
		}

		{
			unsigned w;
			size_t i;

			w = 0;

			for (i = 0; i < new->u.vlist.n; i++) {
				if (new->u.vlist.a[i]->w > w) {
					w = new->u.vlist.a[i]->w;
				}
			}

			new->w = w + 6;
		}

		{
			unsigned a;
			size_t i;

			i = 0;

			/*
			 * Alt lists hang below the line.
			 * The ascender of this node is the ascender of just the first list item
			 * because the first item is at the top of the list, plus the height of
			 * the skip node above that.
			 */

			a = 0;

			if (node->type == NODE_ALT_SKIPPABLE) {
				assert(new->u.vlist.n > i);
				assert(new->u.vlist.a[i] != NULL);
				assert(new->u.vlist.a[i]->type == TNODE_SKIP);
				assert(new->u.vlist.a[i]->a + new->u.vlist.a[i]->d == 1);

				/* arrows are more helpful here */
				new->u.vlist.a[i]->type = TNODE_ARROW;
				new->u.vlist.a[i]->w = 1;

				a += new->u.vlist.a[i]->a + new->u.vlist.a[i]->d + 1;
				i++;
			}

			if (new->u.vlist.n > i) {
				assert(new->u.vlist.a[i] != NULL);

				a += new->u.vlist.a[i]->a;
			}

			new->a = a;
		}

		{
			new->u.vlist.o = 0;

			if (node->type == NODE_ALT_SKIPPABLE) {
				new->u.vlist.o += 1; /* one alt */
			}
		}

		{
			unsigned d;
			size_t i;

			d = 0;

			for (i = 0; i < new->u.vlist.n; i++) {
				d += 1 + new->u.vlist.a[i]->a + new->u.vlist.a[i]->d;
			}

			new->d = d - new->a - 1;
		}

		{
			size_t i;

			for (i = 0; i < new->u.vlist.n; i++) {
				enum tline z;

				int sameline  = i == new->u.vlist.o;
				int aboveline = i <  new->u.vlist.o;
				int belowline = i >  new->u.vlist.o;
				int firstalt  = i == 0;
				int lastalt   = i == new->u.vlist.n - 1;

				if (sameline && new->u.vlist.n > 1 && lastalt) {
					z = TLINE_A;
				} else if (firstalt && aboveline) {
					z = TLINE_B;
				} else if (firstalt && sameline) {
					z = TLINE_C;
				} else if (sameline) {
					z = TLINE_D;
				} else if (belowline && i > 0 && lastalt) {
					z = TLINE_E;
				} else if (new->u.vlist.a[i]->type == TNODE_ELLIPSIS) {
					z = TLINE_F;
				} else {
					z = TLINE_G;
				}

				new->u.vlist.b[i] = z;
			}
		}

		break;

	case NODE_SEQ:
		new->type = TNODE_HLIST;
		new->u.hlist = tnode_create_hlist(node->u.seq, rtl, dim);

		{
			unsigned w;
			size_t i;

			w = 0;

			for (i = 0; i < new->u.hlist.n; i++) {
				w += new->u.hlist.a[i]->w + 2;
			}

			new->w = w - 2;
		}

		{
			unsigned a;
			size_t i;

			a = 0;

			for (i = 0; i < new->u.hlist.n; i++) {
				if (new->u.hlist.a[i]->a > a) {
					a = new->u.hlist.a[i]->a;
				}
			}

			new->a = a;
		}

		{
			unsigned d;
			size_t i;

			d = 0;

			for (i = 0; i < new->u.hlist.n; i++) {
				if (new->u.hlist.a[i]->d > d) {
					d = new->u.hlist.a[i]->d;
				}
			}

			new->d = d;
		}

		break;

	case NODE_LOOP:
		new->type = TNODE_VLIST;

		new->u.vlist.n = 2;
		new->u.vlist.a = xmalloc(sizeof *new->u.vlist.a * new->u.vlist.n);
		new->u.vlist.b = xmalloc(sizeof *new->u.vlist.b * new->u.vlist.n);

		new->u.vlist.a[0] = tnode_create_node(node->u.loop.forward,   rtl, dim);
		new->u.vlist.a[1] = tnode_create_node(node->u.loop.backward, !rtl, dim);

		new->u.vlist.b[0] = TLINE_H;
		new->u.vlist.b[1] = TLINE_E;

		if (new->u.vlist.a[1]->type == TNODE_SKIP) {
			/* arrows are helpful when going backwards */
			new->u.vlist.a[1]->type = TNODE_ARROW;
			new->u.vlist.a[1]->w = 1;
		}

		{
			char s[128]; /* XXX */
			const char *label;

			loop_label(node->u.loop.min, node->u.loop.max, s);
			label = esc_literal(s);

			if (strlen(label) != 0) {
				if (new->u.vlist.a[1]->type == TNODE_SKIP || new->u.vlist.a[1]->type == TNODE_ARROW) {
					struct tnode *label_tnode;

					/* if there's nothing to show for the backwards node, put the label there */
					label_tnode = new->u.vlist.a[1];
					label_tnode->type = TNODE_PROSE;
					label_tnode->u.prose = label;
					dim->rule_string(label, &label_tnode->w, &label_tnode->a, &label_tnode->d);
				} else {
					/* TODO: store label somewhere for rendering to display somehow */
					assert(!"unimplemented");
				}
			}
		}

		{
			unsigned w;
			unsigned wf, wb;

			wf = new->u.vlist.a[0]->w;
			wb = new->u.vlist.a[1]->w;

			w = (wf > wb ? wf : wb) + 6;

			new->w = w;
		}

		{
			new->a = new->u.vlist.a[0]->a;
		}

		{
			new->u.vlist.o = 0;
		}

		{
			unsigned d;
			size_t i;

			d = 0;

			for (i = 0; i < new->u.vlist.n; i++) {
				d += 1 + new->u.vlist.a[i]->a + new->u.vlist.a[i]->d;
			}

			new->d = d - new->a - 1;
		}

		/*
		 * An aesthetic optimisation: When the depth of the forward node is large
		 * a loop going beneath it looks bad, because the backward bars are so high.
		 * A height of 2 seems simple enough to potentially render above the line.
		 * We do that only if there isn't already something big above the line.
		 */
		if (new->u.vlist.a[0]->d >= 4) {
			if (new->u.vlist.a[1]->a + new->u.vlist.a[1]->d <= 2) {
				if (new->u.vlist.a[0]->a <= 2 && new->u.vlist.a[0]->d > new->u.vlist.a[0]->a) {
					swap(&new->u.vlist.a[0], &new->u.vlist.a[1]);

					new->u.vlist.b[0] = TLINE_B;
					new->u.vlist.b[1] = TLINE_I;

					new->u.vlist.o = 1;

					new->a += new->u.vlist.a[0]->a + new->u.vlist.a[0]->d + 1;
					new->d -= new->u.vlist.a[0]->a + new->u.vlist.a[0]->d + 1;
				}
			}
		}

		break;
	}

	return new;
}

struct tnode *
rrd_to_tnode(const struct node *node, const struct dim *dim)
{
	struct tnode *n;

	assert(dim != NULL);

	n = tnode_create_node(node, 0, dim);

	return n;
}

