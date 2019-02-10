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
	int x, y;

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
svg_text(struct render_context *ctx, const char *s, const char *class)
{
	const char *p;

	assert(ctx != NULL);
	assert(s != NULL);

	printf("    <text x='%u0' y='%u5' text-anchor='middle'",
		ctx->x, ctx->y);

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
	printf("    <rect x='%d0' y='%d0' height='%u0' width='%u0' rx='%u' ry='%u'",
		(int) ctx->x, (int) ctx->y - 1,
		2, w,
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
	unsigned x;

	x = ctx->x;

	svg_rect(ctx, w, r, class);
	ctx->x += w / 2; /* XXX: either i want floats, or to scale things */
	svg_text(ctx, s, class);

	ctx->x = x + w;
}

static void
svg_label(struct render_context *ctx, const char *s, unsigned w)
{
	unsigned x;

	assert(ctx != NULL);
	assert(s != NULL);

	x = ctx->x;

	ctx->x += w / 2; /* XXX: either i want floats, or to scale things */
	svg_text(ctx, s, "label");

	ctx->x = x + w;
}

static void
svg_ellipsis(struct render_context *ctx, int w, int h)
{
	ctx->x += 1;
	ctx->y -= 1;

	printf("    <line x1='%u0' y1='%u5' x2='%d0' y2='%u5' class='ellipsis'/>",
		ctx->x, ctx->y,
		(int) ctx->x + w, ctx->y + h);

	ctx->x += w;
	ctx->y += 1;
}

static void
svg_use(struct render_context *ctx, const char *id, const char *transform)
{
	printf("    <use xlink:href='#%s' x='%d0' y='%d0'", id, ctx->x, ctx->y);

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

	lhs = (space - n->w) / 2;
	rhs = (space - n->w) - lhs;

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, lhs);
	}
	ctx->x += lhs;

	node_walk_render(n, ctx, base);

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, rhs);
	}
	ctx->x += rhs;

	ctx->y++;
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
	case TILE_BL_N1: tile = TILE_BL; dy = -1; break;
	case TILE_BR_N1: tile = TILE_BR; dy = -1; break;
	case TILE_TR_N1: tile = TILE_TR; dy = -1; break;

	case TILE_TL:
		dy = +1;
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
	case TILE_BL: y =  1; rx = 0; ry = y; break;
	case TILE_TL: y = -1; rx = 0; ry = y; break;
	case TILE_BR: y = -1; rx = 1; ry = 0; break;
	case TILE_TR: y =  1; rx = 1; ry = 0; break;

	case TILE_LINE:
		svg_path_h(&ctx->paths, ctx->x, ctx->y + dy, 1);
		ctx->x += 1;
		return;

	default:
		assert(!"unreached");
	}

	svg_path_q(&ctx->paths, ctx->x, ctx->y + dy, rx, ry, 1, y);
	ctx->x += 1;
}

static void
render_tile_bm(struct render_context *ctx, unsigned u)
{
	unsigned v;

	if (u == 0) {
		/* nothing to draw */
		ctx->x += 1;
		return;
	}

	for (v = u; v != 0; v &= v - 1) {
		render_tile(ctx, v & -v);

		if ((v & v - 1) != 0) {
			ctx->x -= 1;
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
		ctx->y -= n->a;
	}

	x = ctx->x;
	y = ctx->y;

	for (j = 0; j < n->u.vlist.n; j++) {
		ctx->x = x;

		render_tline_outer(ctx, n->u.vlist.b[j], 0);
		render_tline_inner(ctx, n->u.vlist.b[j], 0);

		justify(ctx, n->u.vlist.a[j], n->w - 4, base);

		ctx->y -= 1;
		render_tline_inner(ctx, n->u.vlist.b[j], 1);
		render_tline_outer(ctx, n->u.vlist.b[j], 1);
		ctx->y += 1;

		if (j + 1 < n->u.vlist.n) {
			ctx->y += n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a;
		}
	}


	/* bars above the line */
	if (n->u.vlist.o > 0) {
		unsigned h;

		h = 0;

		for (j = 0; j < n->u.vlist.o; j++) {
			if (j + 1 < n->u.vlist.n) {
				h += n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a + 1;
			}
		}

		ctx->x = x + 1;
		ctx->y = y + 1;

		h -= 2; /* for the tline corner pieces */
		bars(ctx, h, n->w - 2);
	}

	/* bars below the line */
	if (n->u.vlist.n > n->u.vlist.o + 1) {
		unsigned h;

		h = 0;

		for (j = n->u.vlist.o; j < n->u.vlist.n; j++) {
			if (j + 1 < n->u.vlist.n) {
				h += n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a + 1;
			}
		}

		ctx->x = x + 1;
		ctx->y = o + 1;

		h -= 2; /* for the tline corner pieces */
		bars(ctx, h, n->w - 2);
	}

	ctx->x = x + n->w;
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
		node_walk_render(n->u.hlist.a[!n->rtl ? i : n->u.hlist.n - i], ctx, base);

		if (i + 1 < n->u.hlist.n) {
			svg_path_h(&ctx->paths, ctx->x, ctx->y, 2);
			ctx->x += 2;
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
		ctx->x += n->w;
		break;

	case TNODE_ARROW:
		svg_path_h(&ctx->paths, ctx->x, ctx->y, 1);
		svg_use(ctx, n->rtl ? "rrd:arrow-rtl" : "rrd:arrow-ltr", "translate(5 0)");
		ctx->x += n->w;
		break;

	case TNODE_ELLIPSIS:
		/* TODO: 2 looks too long */
		svg_ellipsis(ctx, 0, 2);
		break;

	case TNODE_CI_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w, 8, "literal");
		printf("    <text x='%u5' y='%u5' text-anchor='left' class='ci'>%s</text>\n",
			ctx->x - 2, ctx->y, "&#x29f8;i");
		break;

	case TNODE_CS_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w, 8, "literal");
		break;

	case TNODE_LABEL:
		svg_label(ctx, n->u.label, n->w);
		break;

	case TNODE_RULE:
		if (base != NULL) {
			printf("<a href='%s#%s'>\n", base, n->u.name); /* XXX: escape */
		}
		svg_textbox(ctx, n->u.name, n->w, 0, "rule");
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

	w = node->w + 8;

	ctx.paths = NULL;

	ctx.x = 0;
	ctx.y = node->a;
	svg_use(&ctx, "rrd:station", "scale(-1 1)");
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 2);

	ctx.x = w - 6;
	svg_path_h(&ctx.paths, ctx.x, ctx.y, 2);
	ctx.x += 2;
	svg_use(&ctx, "rrd:station", NULL);

	ctx.x = 2;
	ctx.y = node->a;
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

		printf("    <path d='M%u0 %u0", p->x, p->y);

		do {
			unsigned nx, ny;

			switch (p->type) {
			case PATH_H: printf(" h%d0", p->u.n); break;
			case PATH_V: printf(" v%d0", p->u.n); break;
			case PATH_Q: printf(" q%d0 %d0 %d0 %d0", p->u.q[0], p->u.q[1], p->u.q[2], p->u.q[3]); break;
			}

			svg_path_move(p, &nx, &ny);

			svg_path_remove(&ctx.paths, p);

			p = svg_path_find_following(ctx.paths, nx, ny);
		} while (p != NULL);

		printf("'/>\n");
	}
}

void
svg_dim_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
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

	/* even numbers only, for sake of w / 2 when centering text */
	if (((unsigned) n & 1) == 1) {
		n++;
	}

	*w = n;
	*a = 1;
	*d = 1;
}

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

		a[i] = rrd_to_tnode(rrd, svg_dim_string);

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
	printf("  viewBox='-20 -50 %u0 %u0'\n", w, h); /* TODO */
	printf("  width='%u0' height='%u0'>\n", w, h);
	printf("\n");

	printf("  <style>\n");
	printf("    rect, line, path { stroke-width: 1.5px; stroke: black; fill: transparent; }\n");
	printf("    rect, line, path { stroke-linecap: square; stroke-linejoin: rounded; }\n");
	printf("    path { fill: transparent; }\n");
	printf("    text.literal { font-family: monospace; }\n");
	printf("    line.ellipsis { stroke-dasharray: 4; }\n");
	printf("  </style>\n");
	printf("\n");

	svg_defs();

	z = 0;

	for (i = 0, p = grammar; p; p = p->next, i++) {
		printf("  <g transform='translate(%u0 %u0)'>\n",
			4, z);
		printf("    <text x='-%u0' y='-%u0'>%s:</text>\n",
			4, 2, p->name);

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

