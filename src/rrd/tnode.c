/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../txt.h"
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
tnode_create_comment(const struct tnode *tnode, const char *s,
	const struct dim *dim);

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

static int
isnamed(char c)
{
	/*
	 * These correspond to the values rendered as named
	 * characters in SVG output (i.e. <TAB> and friends),
	 * and as common escapes ("\t" and so on) for text output.
	 */
	switch (c) {
	case '\a':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return 1;

	default:
		return 0;
	}
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
	case TNODE_RTL_ARROW:
	case TNODE_LTR_ARROW:
	case TNODE_ELLIPSIS:
	case TNODE_RULE:
		break;

	case TNODE_CI_LITERAL:
	case TNODE_CS_LITERAL:
		free((void *) n->u.literal.p);
		break;

	case TNODE_PROSE:
		free((void *) n->u.prose);
		break;

	case TNODE_COMMENT:
		free((void *) n->u.comment.s);
		tnode_free((void *) n->u.comment.tnode);
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

	if (node->u.literal.n != 1) {
		return 0;
	}

	*c = (unsigned) node->u.literal.p[0];

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

	i = 0;
	p = list;

/* TODO: how to handle invisible alts? have the corner tiles hidden?
at the moment we render an empty line, which makes sense in seqs but not in alts
*/

	while (p != NULL) {
		unsigned char c;

		if (!char_terminal(p->node, &c)) {
			new.a[i++] = tnode_create_node(p->node, rtl, dim);
			p = p->next;
			continue;
		}

		if (!bm_get(&bm, c)) {
			/* already output */
			p = p->next;
			continue;
		}

		/* start of range */
		lo = bm_next(&bm, hi, 1);
		if (lo > UCHAR_MAX) {
			/* end of list */
			break;
		}

		/* end of range */
		hi = bm_next(&bm, lo, 0);

		if (!isalnum((unsigned char) lo) && isalnum((unsigned char) hi)) {
			new.a[i++] = tnode_create_node(find_node(list, lo), rtl, dim);
			bm_unset(&bm, lo);

			hi = lo;
			p = p->next;
			continue;
		}

		/*
		 * If the range begins or ends on any named item (e.g. <TAB>),
		 * the output would be confusing since it's not immediately
		 * evident which values are included. So we elect to render the
		 * range as invidual elements instead.
		 */
		if (isnamed((unsigned char) lo) || isnamed((unsigned char) hi)) {
			new.a[i++] = tnode_create_node(find_node(list, lo), rtl, dim);
			bm_unset(&bm, lo);

			hi = lo;
			p = p->next;
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
		new.a[!rtl ? i : new.n - i - 1] = tnode_create_node(p->node, rtl, dim);
		i++;
	}

	assert(i == new.n);

	return new;
}

static const char *
times(unsigned n)
{
	const char *a[] = {
		"never",
		"once", "twice",
		"three times", "four times", "five times", "six times",
		"seven times", "eight times", "nine times", "ten times",
		"eleven times", "twelve times"
	};

	if (n > sizeof a / sizeof *a - 1) {
		return NULL;
	}

	return a[n];
}

static size_t
loop_label(unsigned min, unsigned max, char *s)
{
	const char *h;

	assert(s != NULL);
	assert(max >= min || max == 0);

	if (min == 0 && max == 0) {
		*s = '\0';
		return 0;
	}

	if (max == min) {
		if (h = times(max), h != NULL) {
			return sprintf(s, "(%s only)", h);
		} else {
			return sprintf(s, "(%u times)", max);
		}
	}

	if (min == 0) {
		if (h = times(max), h != NULL) {
			return sprintf(s, "(%s at most)", h);
		} else {
			return sprintf(s, "(%u times at most)", max);
		}
	}

	if (max == 0) {
		if (h = times(min), h != NULL) {
			return sprintf(s, "(at least %s)", h);
		} else {
			return sprintf(s, "(at least %u times)", min);
		}
	}

	return sprintf(s, "(%u-%u times)", min, max);
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
tnode_create_comment(const struct tnode *tnode, const char *s,
	const struct dim *dim)
{
	struct tnode *new;
	unsigned w, a, d;

	assert(tnode != NULL);
	assert(s != NULL);
	assert(dim != NULL);

	new = xmalloc(sizeof *new);

	new->type = TNODE_COMMENT;
	new->u.comment.s = s;
	new->u.comment.tnode = tnode;

	/* TODO: place comment above or below, depending on tnode type (or pass in);
	 * store in .comment struct as enum */
	new->w = new->u.comment.tnode->w;
	new->a = new->u.comment.tnode->a;
	new->d = new->u.comment.tnode->d + dim->comment_height;

	dim->rule_string(new->u.comment.s, &w, &a, &d);

	if (new->w < w) {
		new->w = w;
	}

	new->a += a;
	new->d += d;

	return new;
}

static struct tnode *
tnode_create_node(const struct node *node, int rtl, const struct dim *dim)
{
	struct tnode *new;

	assert(dim != NULL);

	new = xmalloc(sizeof *new);

	if (node == NULL) {
		new->type = TNODE_VLIST;
		new->w = 0;
		new->a = 0;
		new->d = 1;

		new->u.vlist.n = 0;
		new->u.vlist.o = 0;
		new->u.vlist.a = NULL;
		new->u.vlist.b = NULL;

		return new;
	}

	switch (node->type) {
	case NODE_CI_LITERAL:
		new->type = TNODE_CI_LITERAL;
		new->u.literal = xtxtdup(&node->u.literal);
		dim->literal_txt(&new->u.literal, &new->w, &new->a, &new->d);
		new->w += dim->literal_padding + dim->ci_marker;
		break;

	case NODE_CS_LITERAL:
		new->type = TNODE_CS_LITERAL;
		new->u.literal = xtxtdup(&node->u.literal);
		dim->literal_txt(&new->u.literal, &new->w, &new->a, &new->d);
		new->w += dim->literal_padding;
		break;

	case NODE_RULE:
		new->type = TNODE_RULE;
		new->u.name = xstrdup(node->u.name);
		dim->rule_string(new->u.name, &new->w, &new->a, &new->d);
		new->w += dim->rule_padding;
		break;

	case NODE_PROSE:
		new->type = TNODE_PROSE;
		new->u.prose = xstrdup(node->u.prose);
		dim->rule_string(new->u.prose, &new->w, &new->a, &new->d);
		new->w += dim->prose_padding;
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
				assert(new->u.vlist.a[i]->type == TNODE_VLIST && new->u.vlist.a[i]->u.vlist.n == 0);
				assert(new->u.vlist.a[i]->a + new->u.vlist.a[i]->d == 1);

				/* arrows are more helpful here */
				new->u.vlist.a[i]->type = rtl ? TNODE_RTL_ARROW : TNODE_LTR_ARROW;
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

				if (new->u.vlist.n == 1) {
					z = TLINE_J;
				} else if (sameline && new->u.vlist.n > 1 && lastalt) {
					z = rtl ? TLINE_A : TLINE_a;
				} else if (firstalt && aboveline) {
					z = TLINE_B;
				} else if (firstalt && sameline) {
					z = rtl ? TLINE_C : TLINE_c;
				} else if (sameline) {
					z = rtl ? TLINE_D : TLINE_d;
				} else if (belowline && i > 0 && lastalt) {
					z = TLINE_E;
				} else if (new->u.vlist.a[i]->type == TNODE_ELLIPSIS) {
					z = TLINE_F;
				} else {
					z = rtl ? TLINE_G : TLINE_g;
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

		new->u.vlist.b[0] = rtl ? TLINE_H : TLINE_h;
		new->u.vlist.b[1] = TLINE_E;

		if (new->u.vlist.a[1]->type == TNODE_VLIST && new->u.vlist.a[1]->u.vlist.n == 0) {
			/* arrows are helpful when going backwards */
			new->u.vlist.a[1]->type = !rtl ? TNODE_RTL_ARROW : TNODE_LTR_ARROW;
			new->u.vlist.a[1]->w = 1;
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
					new->u.vlist.b[1] = rtl ? TLINE_I : TLINE_i;

					new->u.vlist.o = 1;

					new->a += new->u.vlist.a[0]->a + new->u.vlist.a[0]->d + 1;
					new->d -= new->u.vlist.a[0]->a + new->u.vlist.a[0]->d + 1;
				}
			}
		}

		{
			char s[128]; /* XXX */
			const char *label;

			loop_label(node->u.loop.min, node->u.loop.max, s);
			label = xstrdup(s);

			if (strlen(label) != 0) {
				new = tnode_create_comment(new, label, dim);
			}
		}

		break;
	}

/* TODO:
we make a tnode subtree above, and then if it is invisible, replace the entire thing
with a regular skip or arrow or whatever
TODO: option to show invisible nodes

we do this after constructing the node in order to find its width
*/
	if (node->invisible) {
		struct tnode *old;

		old = new;

		new = xmalloc(sizeof *new);

		new->type = TNODE_VLIST;
		new->w = old->w;
		new->a = old->a;
		new->d = old->d;

		new->u.vlist.n = 0;
		new->u.vlist.o = 0;
		new->u.vlist.a = NULL;
		new->u.vlist.b = NULL;

		tnode_free(old);
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

