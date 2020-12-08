/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Railroad Diagram SVG Output
 *
 * Output a SVG diagram of the abstract representation of railroads.
 * The subset of SVG here is intended to suit RFC 7996.
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "../txt.h"
#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"
#include "../rrd/tnode.h"

#include "io.h"
#include "path.h"

extern const char *css_file;

WARN_UNUSED_RESULT
int
cat(const char *in, const char *indent);

struct render_context {
	unsigned x, y;

	struct path *paths;
	const struct ast_rule *grammar;
};

static void node_walk_render(const struct tnode *n,
	struct render_context *ctx, const char *base);

static int
svg_escputc(FILE *f, char c)
{
	const char *name;

	assert(f != NULL);

	switch (c) {
	case '&': return fputs("&amp;", f);
	case '<': return fputs("&lt;", f);
	case '>': return fputs("&gt;", f);

	case '\a': name = "BEL"; break;
	case '\b': name = "BS";  break;
	case '\f': name = "FF";  break;
	case '\n': name = "LF";  break;
	case '\r': name = "CR";  break;
	case '\t': name = "TAB"; break;
	case '\v': name = "VT";  break;

	default:
		if (!isprint((unsigned char) c)) {
			return fprintf(f, "&#x3008;<tspan class='hex'>%02X</tspan>&#x3009;", (unsigned char) c);
		}

		return fprintf(f, "%c", c);
	}

	return fprintf(f, "&#x3008;<tspan class='esc'>%s</tspan>&#x3009;", name);
}

static void
svg_text(struct render_context *ctx, unsigned w, const struct txt *t, const char *class)
{
	unsigned mid;
	size_t i;

	assert(ctx != NULL);
	assert(t != NULL);
	assert(t->p != NULL);

	mid = w / 2;

	printf("    <text x='%u' y='%u' text-anchor='middle'",
		ctx->x + mid, ctx->y + 5);

	if (class != NULL) {
		printf(" class='%s'", class);
	}

	printf(">");

	for (i = 0; i < t->n; i++) {
		svg_escputc(stdout, t->p[i]);
	}

	printf("</text>\n");
}

static void
svg_string(struct render_context *ctx, unsigned w, const char *s, const char *class)
{
	struct txt t;

	assert(ctx != NULL);
	assert(s != NULL);

	t.p = s;
	t.n = strlen(s);

	svg_text(ctx, w, &t, class);
}

static void
svg_rect(struct render_context *ctx, unsigned w, unsigned r,
	const char *class)
{
	printf("    <rect x='%u' y='%u' height='%u' width='%u' rx='%u' ry='%u'",
		ctx->x, ctx->y - 10,
		20, w,
		r, r);

	if (class != NULL) {
		printf(" class='%s'", class);
	}

	printf("/>\n");
}

static void
svg_textbox(struct render_context *ctx, const struct txt *t, unsigned w, unsigned r,
	const char *class)
{
	assert(t != NULL);
	assert(t->p != NULL);

	svg_rect(ctx, w, r, class);
	svg_text(ctx, w, t, class);

	ctx->x += w;
}

static void
svg_prose(struct render_context *ctx, const char *s, unsigned w)
{
	assert(ctx != NULL);
	assert(s != NULL);

	svg_string(ctx, w, s, "prose");

	ctx->x += w;
}

static void
svg_ellipsis(struct render_context *ctx, unsigned w, unsigned h)
{
	ctx->x += 10;
	ctx->y -= 10;

	printf("    <line x1='%u' y1='%u' x2='%u' y2='%u' class='ellipsis'/>",
		ctx->x - 5, ctx->y + 5,
		ctx->x + w - 5, ctx->y + h + 5);

	ctx->x += w;
	ctx->y += 10;
}

static void
svg_arrow(struct render_context *ctx, unsigned x, unsigned y, int rtl)
{
	unsigned h = 6;

	assert(ctx != NULL);

	/* XXX: should be markers, but aren't for RFC 7996 */
	/* 2 for optical correction */
	printf("    <path d='M%d %u l%d %u v%d z' class='arrow'/>\n",
		(int) x + (rtl ? -2 : 2), y, rtl ? 4 : -4, h / 2, -h);
}

static void
centre(unsigned *lhs, unsigned *rhs, unsigned space, unsigned w)
{
	assert(lhs != NULL);
	assert(rhs != NULL);
	assert(space >= w);

	*lhs = (space - w) / 2;
	*rhs = (space - w) - *lhs;
}

static void
justify(struct render_context *ctx, const struct tnode *n, unsigned space,
	const char *base)
{
	unsigned lhs, rhs;

	centre(&lhs, &rhs, space, n->w * 10);

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, lhs);
	}
	if (debug) {
		svg_rect(ctx, lhs, 5, "debug justify");
	}
	ctx->x += lhs;

	node_walk_render(n, ctx, base);

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, rhs);
	}
	if (debug) {
		svg_rect(ctx, rhs, 5, "debug justify");
	}
	ctx->x += rhs;
}

static void
bars(struct render_context *ctx, unsigned n, unsigned w)
{
	svg_path_v(&ctx->paths, ctx->x, ctx->y, n);
	ctx->x += w;
	svg_path_v(&ctx->paths, ctx->x, ctx->y, n);
}

enum tile {
	TILE_BL = 1 << 0, /* `- bottom left */
	TILE_TL = 1 << 1, /* .- top left */
	TILE_BR = 1 << 2, /* -' bottom right */
	TILE_TR = 1 << 3, /* -. top right */

	TILE_LINE = 1 << 4, /* horizontal line */

	TILE_BL_N1 = 1 << 5,
	TILE_BR_N1 = 1 << 6,
	TILE_TR_N1 = 1 << 7
};

static void
render_tile(struct render_context *ctx, enum tile tile)
{
	int y, dy;
	int rx, ry;

	switch (tile) {
	case TILE_BL_N1: tile = TILE_BL; dy = -10; break;
	case TILE_BR_N1: tile = TILE_BR; dy = -10; break;
	case TILE_TR_N1: tile = TILE_TR; dy = -10; break;

	case TILE_TL:
		dy = 10;
		break;

	case TILE_BR:
	case TILE_TR:
	case TILE_LINE:
		dy = 0;
		break;

	case TILE_BL:
	default:
		assert(!"unreached");
		break;
	}

	switch (tile) {
	case TILE_BL: y =  10; rx =  0; ry = y; break;
	case TILE_TL: y = -10; rx =  0; ry = y; break;
	case TILE_BR: y = -10; rx = 10; ry = 0; break;
	case TILE_TR: y =  10; rx = 10; ry = 0; break;

	case TILE_LINE:
		svg_path_h(&ctx->paths, ctx->x, ctx->y + dy, 10);
		ctx->x += 10;
		return;

	default:
		assert(!"unreached");
	}

	if (debug) {
		char s[16];
		struct txt t;

		snprintf(s, sizeof s, "%d", tile);

		t.p = s;
		t.n = strlen(s);
		svg_textbox(ctx, &t, 10, 0, "debug tile");
		ctx->x -= 10;
	}

	svg_path_q(&ctx->paths, ctx->x, ctx->y + dy, rx, ry, 10, y);
	ctx->x += 10;
}

static void
render_tile_bm(struct render_context *ctx, unsigned u)
{
	unsigned v;

	if (u == 0) {
		/* nothing to draw */
		ctx->x += 10;
		return;
	}

	for (v = u; v != 0; v &= v - 1) {
		render_tile(ctx, v & -v);

		if ((v & (v - 1)) != 0) {
			ctx->x -= 10;
		}
	}
}

static void
render_tline_inner(struct render_context *ctx, enum tline tline, int rhs)
{
	unsigned u[2];

	assert(ctx != NULL);

	switch (tline) {
	case TLINE_A:
	case TLINE_a: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_B: u[0] = TILE_TL;    u[1] = TILE_TR;   break;
	case TLINE_C:
	case TLINE_c: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_D:
	case TLINE_d: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_E: u[0] = TILE_BL_N1; u[1] = TILE_BR;   break;
	case TLINE_F: u[0] = 0;          u[1] = 0;         break;
	case TLINE_G:
	case TLINE_g: u[0] = TILE_BL_N1; u[1] = TILE_BR;   break;

	case TLINE_H:
	case TLINE_h: u[0] = TILE_LINE | TILE_TL;    u[1] = TILE_LINE | TILE_TR; break;
	case TLINE_I:
	case TLINE_i: u[0] = TILE_LINE | TILE_BL_N1; u[1] = TILE_LINE | TILE_BR; break;

	case TLINE_J: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;

	default: u[0] = 0; u[1] = 0; break;
	}

	render_tile_bm(ctx, u[rhs]);
}

static void
render_tline_outer(struct render_context *ctx, enum tline tline, int rhs)
{
	unsigned u[2];

	assert(ctx != NULL);

	switch (tline) {
	case TLINE_A:
	case TLINE_a: u[0] = TILE_LINE | TILE_BR; u[1] = TILE_LINE | TILE_BL_N1; break;
	case TLINE_C:
	case TLINE_c: u[0] = TILE_LINE | TILE_TR; u[1] = TILE_LINE | TILE_TL;    break;
	case TLINE_D:
	case TLINE_d: u[0] = TILE_LINE | TILE_BR | TILE_TR; u[1] = TILE_LINE | TILE_BL_N1 | TILE_TL; break;
	case TLINE_H:
	case TLINE_h: u[0] = TILE_LINE;           u[1] = TILE_LINE; break;
	case TLINE_I:
	case TLINE_i: u[0] = TILE_LINE;           u[1] = TILE_LINE; break;
	case TLINE_J: u[0] = TILE_LINE;           u[1] = TILE_LINE; break;

	default: u[0] = 0; u[1] = 0; break;
	}

	render_tile_bm(ctx, u[rhs]);
}

static void
render_vlist(const struct tnode *n,
	struct render_context *ctx, const char *base)
{
	unsigned x, o, y;
	size_t j;

	assert(n != NULL);
	assert(n->type == TNODE_VLIST);
	assert(ctx != NULL);

	o = ctx->y;

	assert(n->u.vlist.o <= 1); /* currently only implemented for one node above the line */
	if (n->u.vlist.o == 1) {
		ctx->y -= n->a * 10;
	}

	x = ctx->x;
	y = ctx->y;


	/*
	 * A vlist of 0 items is a special case, meaning to draw
	 * a horizontal line only.
	 */
	if (n->u.vlist.n == 0 && n->w > 0) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, n->w * 10);
	} else for (j = 0; j < n->u.vlist.n; j++) {
		ctx->x = x;

		render_tline_outer(ctx, n->u.vlist.b[j], 0);
		render_tline_inner(ctx, n->u.vlist.b[j], 0);

		justify(ctx, n->u.vlist.a[j], n->w * 10 - 40, base);

		render_tline_inner(ctx, n->u.vlist.b[j], 1);
		render_tline_outer(ctx, n->u.vlist.b[j], 1);

		ctx->y += 10;

		if (j + 1 < n->u.vlist.n) {
			ctx->y += (n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a) * 10;
		}
	}

	/* bars above the line */
	if (n->u.vlist.o > 0) {
		unsigned h;

		h = 0;

		for (j = 0; j < n->u.vlist.o; j++) {
			if (j + 1 < n->u.vlist.n) {
				h += (n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a + 1) * 10;
			}
		}

		ctx->x = x + 10;
		ctx->y = y + 10;

		h -= 20; /* for the tline corner pieces */
		bars(ctx, h, n->w * 10 - 20);
	}

	/* bars below the line */
	if (n->u.vlist.n > n->u.vlist.o + 1) {
		unsigned h;

		h = 0;

		for (j = n->u.vlist.o; j < n->u.vlist.n; j++) {
			if (j + 1 < n->u.vlist.n) {
				h += (n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a + 1) * 10;
			}
		}

		ctx->x = x + 10;
		ctx->y = o + 10;

		h -= 20; /* for the tline corner pieces */
		bars(ctx, h, n->w * 10 - 20);
	}

	ctx->x = x + n->w * 10;
	ctx->y = o;
}

static void
render_hlist(const struct tnode *n,
	struct render_context *ctx, const char *base)
{
	size_t i;

	assert(n != NULL);
	assert(n->type == TNODE_HLIST);
	assert(ctx != NULL);

	for (i = 0; i < n->u.hlist.n; i++) {
		node_walk_render(n->u.hlist.a[i], ctx, base);

		if (i + 1 < n->u.hlist.n) {
			svg_path_h(&ctx->paths, ctx->x, ctx->y, 20);
			ctx->x += 20;
		}
	}
}

static void
node_walk_render(const struct tnode *n,
	struct render_context *ctx, const char *base)
{
	assert(ctx != NULL);

	if (debug) {
		svg_rect(ctx, n->w * 10, 2, "debug node");
	}

	switch (n->type) {
	case TNODE_RTL_ARROW:
		svg_path_h(&ctx->paths, ctx->x, ctx->y, 10);
		svg_arrow(ctx, ctx->x + n->w * 5, ctx->y, 1);
		ctx->x += n->w * 10;
		break;

	case TNODE_LTR_ARROW:
		svg_path_h(&ctx->paths, ctx->x, ctx->y, 10);
		svg_arrow(ctx, ctx->x + n->w * 5, ctx->y, 0);
		ctx->x += n->w * 10;
		break;

	case TNODE_ELLIPSIS:
		svg_ellipsis(ctx, 0, (n->a + n->d + 1) * 10);
		break;

	case TNODE_CI_LITERAL:
		svg_textbox(ctx, &n->u.literal, n->w * 10, 8, "literal");
		printf("    <text x='%u' y='%u' text-anchor='left' class='ci'>%s</text>\n",
			ctx->x - 20 + 5, ctx->y + 5, "&#x29f8;i");
		break;

	case TNODE_CS_LITERAL:
		svg_textbox(ctx, &n->u.literal, n->w * 10, 8, "literal");
		break;

	case TNODE_PROSE:
		svg_prose(ctx, n->u.prose, n->w * 10);
		break;

	case TNODE_COMMENT: {
		unsigned offset = 5;

		ctx->y += n->d * 10;

		/* TODO: - 5 again for loops with a backwards skip (because they're short) */
		if (n->u.comment.tnode->type == TNODE_VLIST
		&& n->u.comment.tnode->u.vlist.o == 0
		&& n->u.comment.tnode->u.vlist.n == 2
		&& ((n->u.comment.tnode->u.vlist.a[1]->type == TNODE_VLIST && n->u.comment.tnode->u.vlist.a[1]->u.vlist.n == 0) || n->u.comment.tnode->u.vlist.a[1]->type == TNODE_RTL_ARROW || n->u.comment.tnode->u.vlist.a[1]->type == TNODE_LTR_ARROW)) {
			offset += 10;
		}

		ctx->y -= offset; /* off-grid */
		svg_string(ctx, n->w * 10, n->u.comment.s, "comment");
		ctx->y += offset;
		ctx->y -= n->d * 10;
		justify(ctx, n->u.comment.tnode, n->w * 10, base);
		break;
	}

	case TNODE_RULE: {
		/*
		 * We don't make something a link if it doesn't have a destination in
		 * the same document. That is, rules need not be defined in the same
		 * grammar. 
		 */
		int dest_exists = !!ast_find_rule(ctx->grammar, n->u.name);

		if (base != NULL && dest_exists) {
			printf("    <a href='%s#%s'>\n", base, n->u.name); /* XXX: escape */
		}
		{
			struct txt t;

			t.p = n->u.name;
			t.n = strlen(n->u.name);

			svg_textbox(ctx, &t, n->w * 10, 0, "rule");
		}
		if (base != NULL && dest_exists) {
			printf("    </a>\n");
		}
		break;
	}

	case TNODE_VLIST:
		/* TODO: .n == 0 skips under loop alts are too close to the line */
		render_vlist(n, ctx, base);
		break;

	case TNODE_HLIST:
		render_hlist(n, ctx, base);
		break;
	}
}

void
svg_render_station(unsigned x, unsigned y)
{
	unsigned gap = 4;
	unsigned h = 12;

	/* .5 to overlap the line width */
	printf("    <path d='M%u.5 %u v%u m %u 0 v%d' class='station'/>\n",
		x, y - h / 2, h, gap, -h);
}

void
svg_render_rule(const struct tnode *node, const char *base,
	const struct ast_rule *grammar)
{
	struct render_context ctx;
	unsigned w;

	w = (node->w + 8) * 10;

	/*
	 * Just to save passing it along through every production;
	 * this is only used informatively, and has nothing to do
	 * with the structure of rendering.
	 */
	ctx.grammar = grammar;

	ctx.paths = NULL;

	ctx.x = 5;
	ctx.y = node->a * 10 + 10;
	svg_render_station(ctx.x, ctx.y);
	ctx.x = 10;
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 20);

	ctx.x = w - 50;
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 20);
	ctx.x += 20;
	svg_render_station(ctx.x, ctx.y);

	ctx.x = 30;
	ctx.y = node->a * 10 + 10;
	node_walk_render(node, &ctx, base);

	/*
	 * Consolidate adjacent nodes of the same type.
	 */
	svg_path_consolidate(&ctx.paths);

	/*
	 * Next we consolidate on-the-fly to render a single path segment
	 * for a individual path with differently-typed items which connect
	 * in a sequence. This is just an effort to produce tidy markup.
	 */

	while (ctx.paths != NULL) {
		struct path *p;

		p = svg_path_find_start(ctx.paths);

		printf("    <path d='M%d %d", p->x, p->y);

		do {
			unsigned nx, ny;

			switch (p->type) {
			case PATH_H: printf(" h%d", p->u.n); break;
			case PATH_V: printf(" v%d", p->u.n); break;
			case PATH_Q: printf(" q%d %d %d %d", p->u.q[0], p->u.q[1], p->u.q[2], p->u.q[3]); break;
			}

			svg_path_move(p, &nx, &ny);

			svg_path_remove(&ctx.paths, p);

			/* consolidate only when not debugging */
			if (debug) {
				break;
			}

			p = svg_path_find_following(ctx.paths, nx, ny);
		} while (p != NULL);

		printf("'/>\n");
	}
}

static void
dim_prop_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	const char *p;
	double n;

	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	n = 0.0;

	/* estimate at proportional width */

	for (p = s; *p != '\0'; p++) {
		switch (tolower((unsigned char) *p)) {
		case '|':
			n += 0.3;
			break;

		case 't':
			n += 0.45;
			break;

		case 'f':
		case 'i':
		case 'j':
		case 'l':
			n += 0.5;
			break;

		case '(': case ')':
		case 'I':
			n += 0.55;
			break;

		case ' ':
			n += 0.6;
			break;

		case 'm':
			n += 1.25;
			break;

		case 'w':
			n += 1.2;
			break;

		case '1':
			n += 0.75;
			break;

		default:
			n += 0.8;
			break;
		}

		if (isupper((unsigned char) *p)) {
			n += 0.25;
		}
	}

	n = ceil(n);

	/* even numbers only, for sake of visual rhythm */
	if (((unsigned) n & 1) == 1) {
		n++;
	}

	*w = n + 1;
	*a = 1;
	*d = 1;
}

static void
dim_mono_txt(const struct txt *t, unsigned *w, unsigned *a, unsigned *d)
{
	size_t i;
	double n;

	assert(t != NULL);
	assert(t->p != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	n = 0.0;

	for (i = 0; i < t->n; i++) {
		if (t->p[i] == '\t' || t->p[i] == '\a') {
			n += 4.00;
			continue;
		}

		if (!isprint((unsigned char) t->p[i])) {
			n += 2.93; /* <XY> */
			continue;
		}

		n += 1.0;
	}

	n = ceil(n);

	*w = n + 1;
	*a = 1;
	*d = 1;
}

struct dim svg_dim = {
	dim_mono_txt,
	dim_prop_string,
	0,
	0,
	0,
	1,
	2,
	0
};

WARN_UNUSED_RESULT
int
svg_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;
	struct tnode **a;
	unsigned z;
	unsigned w, h;
	unsigned i, n;

	n = 0;
	for (p = grammar; p; p = p->next) {
		n++;
	}

	/*
	 * We store all tnodes for sake of calculating the viewport only;
	 * it's a shame this needs to be provided ahead of rendering each
	 * tnode, else we could do that on the fly.
	 */

	a = xmalloc(sizeof *a * n);

	w = 0;
	h = 0;

	for (i = 0, p = grammar; p; p = p->next, i++) {
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return 0;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		a[i] = rrd_to_tnode(rrd, &svg_dim);

		if (a[i]->w > w) {
			w = a[i]->w;
		}
		h += a[i]->a + a[i]->d + 6;

		node_free(rrd);
	}

	w += 12;
	h += 5;

	printf("<?xml version='1.0' encoding='utf-8'?>\n");
	printf("<svg\n");
	printf("  xmlns='http://www.w3.org/2000/svg'\n");
	printf("  xmlns:xlink='http://www.w3.org/1999/xlink'\n");
	printf("\n");
	printf("  width='%u0' height='%u'>\n", w, h * 10 + 60);
	printf("\n");

	printf("  <style>\n");

	printf("    rect, line, path { stroke-width: 1.5px; stroke: black; fill: transparent; }\n");
	printf("    rect, line, path { stroke-linecap: square; stroke-linejoin: rounded; }\n");

	if (debug) {
		printf("    rect.debug { stroke: none; opacity: 0.75; }\n");
		printf("    rect.debug.tile { fill: #cccccc; }\n");
		printf("    rect.debug.node { fill: transparent; stroke-width: 1px; stroke: #ccccff; stroke-dasharray: 2 3; }\n");
		printf("    rect.debug.justify { fill: #ccccff; }\n");
		printf("    text.debug.tile { opacity: 0.3; font-family: monospace; font-weight: bold; stroke: none; }\n");
	}

	printf("    path { fill: transparent; }\n");
	printf("    text.literal { font-family: monospace; }\n");
	printf("    line.ellipsis { stroke-dasharray: 1 3.5; }\n");
	printf("    tspan.hex { font-family: monospace; font-size: 90%%; }\n");
	printf("    path.arrow { fill: black; }\n");

	if (css_file != NULL) {
		if (!cat(css_file, "    ")) {
			return 0;
		}
	}

	printf("  </style>\n");
	printf("\n");

	z = 0;

	for (i = 0, p = grammar; p; p = p->next, i++) {
		printf("  <g transform='translate(%u %u)'>\n",
			40, z * 10 + 50);
		printf("    <text x='%d' y='%d'>%s:</text>\n",
			-30, -10, p->name);

		svg_render_rule(a[i], NULL, grammar);

		printf("  </g>\n");
		printf("\n");

		z += a[i]->a + a[i]->d + 6;
	}

	for (i = 0, p = grammar; p; p = p->next, i++) {
		tnode_free(a[i]);
	}

	free(a);

	printf("</svg>\n");
	return 1;
}

