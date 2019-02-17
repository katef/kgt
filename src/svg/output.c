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

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

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

struct render_context {
	int x, y; /* in svg units */

	struct path *paths;
};

static void node_walk_render(const struct tnode *n,
	struct render_context *ctx, const char *base);

int
xml_escputc(FILE *f, char c)
{
	assert(f != NULL);

	switch (c) {
	case '&': return fputs("&amp;", f);
	case '<': return fputs("&lt;", f);
	case '>': return fputs("&gt;", f);

	default:
		break;
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "&#x%02x;", (unsigned char) c);
	}

	return fprintf(f, "%c", c);
}

static void
svg_text(struct render_context *ctx, unsigned w, const char *s, const char *class)
{
	const char *p;
	unsigned mid;

	assert(ctx != NULL);
	assert(s != NULL);

	mid = w / 2;

	printf("    <text x='%u' y='%u' text-anchor='middle'",
		ctx->x + mid, ctx->y + 5);

	if (class != NULL) {
		printf(" class='%s'", class);
	}

	printf(">");

	for (p = s; *p != '\0'; p++) {
		xml_escputc(stdout, *p);
	}

	printf("</text>\n");
}

static void
svg_rect(struct render_context *ctx, unsigned w, unsigned r,
	const char *class)
{
	printf("    <rect x='%d' y='%d' height='%u' width='%u' rx='%u' ry='%u'",
		ctx->x, ctx->y - 10,
		20, w,
		r, r);

	if (class != NULL) {
		printf(" class='%s'", class);
	}

	printf("/>\n");
}

static void
svg_textbox(struct render_context *ctx, const char *s, unsigned w, unsigned r,
	const char *class)
{
	svg_rect(ctx, w, r, class);
	svg_text(ctx, w, s, class);

	ctx->x += w;
}

static void
svg_prose(struct render_context *ctx, const char *s, unsigned w)
{
	assert(ctx != NULL);
	assert(s != NULL);

	svg_text(ctx, w, s, "prose");

	ctx->x += w;
}

static void
svg_ellipsis(struct render_context *ctx, int w, int h)
{
	ctx->x += 10;
	ctx->y -= 10;

	printf("    <line x1='%d' y1='%d' x2='%d' y2='%d' class='ellipsis'/>",
		ctx->x - 5, ctx->y + 5,
		ctx->x + w - 5, ctx->y + h + 5);

	ctx->x += w;
	ctx->y += 10;
}

static void
svg_use(struct render_context *ctx, const char *id, const char *transform)
{
	printf("    <use xlink:href='#%s' x='%d' y='%d'", id, ctx->x, ctx->y);

	if (transform != NULL) {
		printf(" transform='%s'", transform);
	}

	printf("/>\n");
}

static void
justify(struct render_context *ctx, const struct tnode *n, int space,
	const char *base)
{
	unsigned lhs, rhs;

	lhs = (space - n->w * 10) / 2;
	rhs = (space - n->w * 10) - lhs;

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, lhs);
	}
	ctx->x += lhs;

	node_walk_render(n, ctx, base);

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, rhs);
	}
	ctx->x += rhs;

	ctx->y += 10;
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
	case TLINE_A: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_B: u[0] = TILE_TL;    u[1] = TILE_TR;   break;
	case TLINE_C: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_D: u[0] = TILE_LINE;  u[1] = TILE_LINE; break;
	case TLINE_E: u[0] = TILE_BL_N1; u[1] = TILE_BR;   break;
	case TLINE_F: u[0] = 0;          u[1] = 0;         break;
	case TLINE_G: u[0] = TILE_BL_N1; u[1] = TILE_BR;   break;

	case TLINE_H: u[0] = TILE_LINE | TILE_TL;    u[1] = TILE_LINE | TILE_TR; break;
	case TLINE_I: u[0] = TILE_LINE | TILE_BL_N1; u[1] = TILE_LINE | TILE_BR; break;

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
	case TLINE_A: u[0] = TILE_LINE | TILE_BR; u[1] = TILE_LINE | TILE_BL_N1; break;
	case TLINE_C: u[0] = TILE_LINE | TILE_TR; u[1] = TILE_LINE | TILE_TL;    break;
	case TLINE_D: u[0] = TILE_LINE | TILE_BR | TILE_TR; u[1] = TILE_LINE | TILE_BL_N1 | TILE_TL; break;
	case TLINE_H: u[0] = TILE_LINE;           u[1] = TILE_LINE; break;
	case TLINE_I: u[0] = TILE_LINE;           u[1] = TILE_LINE; break;

	default: u[0] = 0; u[1] = 0; break;
	}

	render_tile_bm(ctx, u[rhs]);
}

static void
render_vlist(const struct tnode *n,
	struct render_context *ctx, const char *base)
{
	int x, o, y;
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

	for (j = 0; j < n->u.vlist.n; j++) {
		ctx->x = x;

		render_tline_outer(ctx, n->u.vlist.b[j], 0);
		render_tline_inner(ctx, n->u.vlist.b[j], 0);

		justify(ctx, n->u.vlist.a[j], n->w * 10 - 40, base);

		ctx->y -= 10;
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
		node_walk_render(n->u.hlist.a[!n->rtl ? i : n->u.hlist.n - i - 1], ctx, base);

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

	switch (n->type) {
	case TNODE_SKIP:
		/* TODO: skips under loop alts are too close to the line */
		ctx->x += n->w * 10;
		break;

	case TNODE_ARROW:
		svg_path_h(&ctx->paths, ctx->x, ctx->y, 10);
		svg_use(ctx, n->rtl ? "rrd:arrow-rtl" : "rrd:arrow-ltr", "translate(5 0)");
		ctx->x += n->w * 10;
		break;

	case TNODE_ELLIPSIS:
		svg_ellipsis(ctx, 0, (n->a + n->d + 1) * 10);
		break;

	case TNODE_CI_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w * 10, 8, "literal");
		printf("    <text x='%d' y='%d' text-anchor='left' class='ci'>%s</text>\n",
			ctx->x - 20 + 5, ctx->y + 5, "&#x29f8;i");
		break;

	case TNODE_CS_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w * 10, 8, "literal");
		break;

	case TNODE_PROSE:
		svg_prose(ctx, n->u.prose, n->w * 10);
		break;

	case TNODE_RULE:
		if (base != NULL) {
			printf("<a href='%s#%s'>\n", base, n->u.name); /* XXX: escape */
		}
		svg_textbox(ctx, n->u.name, n->w * 10, 0, "rule");
		if (base != NULL) {
			printf("</a>\n");
		}
		break;

	case TNODE_VLIST:
		render_vlist(n, ctx, base);
		break;

	case TNODE_HLIST:
		render_hlist(n, ctx, base);
		break;
	}
}

void
svg_render_rule(const struct tnode *node, const char *base)
{
	struct render_context ctx;
	unsigned w;

	w = (node->w + 8) * 10;

	ctx.paths = NULL;

	ctx.x = 0;
	ctx.y = node->a * 10;
	svg_use(&ctx, "rrd:station", "scale(-1 1)");
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 20);

	ctx.x = w - 60;
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 20);
	ctx.x += 20;
	svg_use(&ctx, "rrd:station", NULL);

	ctx.x = 20;
	ctx.y = node->a * 10;
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
dim_mono_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = strlen(s) + 1;
	*a = 1;
	*d = 1;
}

struct dim svg_dim = {
	dim_mono_string,
	dim_prop_string,
	0,
	0,
	0,
	2,
	0
};

void
svg_defs(void)
{
	printf("  <defs>\n");
	printf("    <g id='rrd:station'>\n");
	printf("      <path d='M.5 -6 v12 m 5 0 v-12' class='station'/>\n"); /* .5 to overlap the line width */
	printf("    </g>\n");
	printf("\n");

	/* XXX: should be markers, but aren't for RFC 7996 */
	printf("    <g id='rrd:arrow-ltr'>\n");
	printf("      <polyline points='2,-4 10,0 2,4' class='arrow'/>\n"); /* 2 for optical correction */
	printf("    </g>\n");
	printf("    <g id='rrd:arrow-rtl' transform='scale(-1 1) translate(-10 0)'>\n");
	printf("      <use xlink:href='#rrd:arrow-ltr'/>");
	printf("    </g>\n");
	printf("  </defs>\n");
	printf("\n");
}

void
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
			return;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		a[i] = rrd_to_tnode(rrd, &svg_dim);

		if (a[i]->w > w) {
			w = a[i]->w;
		}
		h += a[i]->a + a[i]->d + 5;

		node_free(rrd);
	}

	w += 12;
	h += 5;

	printf("<svg\n");
	printf("  xmlns='http://www.w3.org/2000/svg'\n");
	printf("  xmlns:xlink='http://www.w3.org/1999/xlink'\n");
	printf("\n");
	printf("  viewBox='-20 -50 %u %u'\n", w * 10, h * 10); /* TODO */
	printf("  width='%u0' height='%u'>\n", w, h * 10);
	printf("\n");

	printf("  <style>\n");
	printf("    rect, line, path { stroke-width: 1.5px; stroke: black; fill: transparent; }\n");
	printf("    rect, line, path { stroke-linecap: square; stroke-linejoin: rounded; }\n");
	printf("    path { fill: transparent; }\n");
	printf("    text.literal { font-family: monospace; }\n");
	printf("    line.ellipsis { stroke-dasharray: 1 3.5; }\n");
	printf("  </style>\n");
	printf("\n");

	svg_defs();

	z = 0;

	for (i = 0, p = grammar; p; p = p->next, i++) {
		printf("  <g transform='translate(%d %u)'>\n",
			40, z * 10);
		printf("    <text x='-%d' y='-%d'>%s:</text>\n",
			40, 20, p->name);

		svg_render_rule(a[i], NULL);

		printf("  </g>\n");
		printf("\n");

		z += a[i]->a + a[i]->d + 5;
	}

	for (i = 0, p = grammar; p; p = p->next, i++) {
		tnode_free(a[i]);
	}

	free(a);

	printf("</svg>\n");
}

