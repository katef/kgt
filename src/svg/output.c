/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Railroad Diagram SVG Output
 *
 * Output a SVG diagram of the abstract representation of railroads
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

static void node_walk_render(const struct tnode *n, struct render_context *ctx);

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
svg_text(struct render_context *ctx, const char *s)
{
	const char *p;

	assert(ctx != NULL);
	assert(s != NULL);

	printf("    <text x='%u0' y='%u5' text-anchor='middle'>",
		ctx->x, ctx->y);

	for (p = s; *p != '\0'; p++) {
		xml_escputc(stdout, *p);
	}

	printf("</text>\n");
}

static void
svg_rect(struct render_context *ctx, unsigned w, unsigned r)
{
	printf("    <rect x='%d0' y='%d0' height='%u0' width='%u0' rx='%u' ry='%u'",
		(int) ctx->x, (int) ctx->y - 1,
		2, w,
		r, r);

	if (r > 0) {
		printf(" class='rounded'");
	}

	printf("/>\n");
}

static void
svg_textbox(struct render_context *ctx, const char *s, unsigned w, unsigned r)
{
	unsigned x;

	x = ctx->x;

	svg_rect(ctx, w, r);
	ctx->x += w / 2; /* XXX: either i want floats, or to scale things */
	svg_text(ctx, s);

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
	svg_text(ctx, s);

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
justify(struct render_context *ctx, const struct tnode *n, int space)
{
	unsigned lhs, rhs;

	if (n->type == TNODE_ARROW) {
		((struct tnode *) n)->w = space; /* XXX: hacky */
	}

	lhs = (space - n->w) / 2;
	rhs = (space - n->w) - lhs;

	if (n->type != TNODE_ELLIPSIS) {
		svg_path_h(&ctx->paths, ctx->x, ctx->y, lhs);
	}
	ctx->x += lhs;

	node_walk_render(n, ctx);

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
render_alt(const struct tnode *n, struct render_context *ctx)
{
	int x, o, y;
	size_t j;

	assert(n != NULL);
	assert(n->type == TNODE_ALT);
	assert(ctx != NULL);

	o = ctx->y;

	assert(n->o <= 1); /* currently only implemented for one node above the line */
	if (n->o == 1) {
		ctx->y -= n->a;
	}

	x = ctx->x;
	y = ctx->y;

	for (j = 0; j < n->u.alt.n; j++) {
		ctx->x = x;

		render_tline_outer(ctx, n->u.alt.b[j], 0);
		render_tline_inner(ctx, n->u.alt.b[j], 0);

		justify(ctx, n->u.alt.a[j], n->w - 4);

		ctx->y -= 1;
		render_tline_inner(ctx, n->u.alt.b[j], 1);
		render_tline_outer(ctx, n->u.alt.b[j], 1);
		ctx->y += 1;

		if (j + 1 < n->u.alt.n) {
			ctx->y += n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a;
		}
	}


	/* bars above the line */
	if (n->o > 0) {
		unsigned h;

		h = 0;

		for (j = 0; j < n->o; j++) {
			if (j + 1 < n->u.alt.n) {
				h += n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a + 1;
			}
		}

		ctx->x = x + 1;
		ctx->y = y + 1;

		h -= 2; /* for the tline corner pieces */
		bars(ctx, h, n->w - 2);
	}

	/* bars below the line */
	if (n->u.alt.n > n->o + 1) {
		unsigned h;

		h = 0;

		for (j = n->o; j < n->u.alt.n; j++) {
			if (j + 1 < n->u.alt.n) {
				h += n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a + 1;
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
render_seq(const struct tnode *n, struct render_context *ctx)
{
	size_t i;

	assert(n != NULL);
	assert(n->type == TNODE_SEQ);
	assert(ctx != NULL);

	for (i = 0; i < n->u.seq.n; i++) {
		node_walk_render(n->u.seq.a[!n->rtl ? i : n->u.seq.n - i], ctx);

		if (i + 1 < n->u.seq.n) {
			svg_path_h(&ctx->paths, ctx->x, ctx->y, 2);
			ctx->x += 2;
		}
	}
}

static void
node_walk_render(const struct tnode *n, struct render_context *ctx)
{
	assert(ctx != NULL);

	switch (n->type) {
	case TNODE_SKIP:
		/* TODO: skips under loop alts are too close to the line */
		ctx->x += n->w;
		break;

	case TNODE_ARROW:
		printf("    <path d='M%u0 %u0 h%.1f H%u0' class='arrow %s'/>\n",
			ctx->x, ctx->y, (float) n->w / 0.2, ctx->x + n->w,
			n->rtl ? "rtl" : "ltr");
		ctx->x += n->w;
		break;

	case TNODE_ELLIPSIS:
		/* TODO: 2 looks too long */
		svg_ellipsis(ctx, 0, 2);
		break;

	case TNODE_CI_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w, 8);
		printf("    <text x='%u5' y='%u5' text-anchor='left' class='ci'>%s</text>\n",
			ctx->x - 2, ctx->y, "&#x29f8;i");
		break;

	case TNODE_CS_LITERAL:
		svg_textbox(ctx, n->u.literal, n->w, 8);
		break;

	case TNODE_LABEL:
		svg_label(ctx, n->u.label, n->w);
		break;

	case TNODE_RULE:
		svg_textbox(ctx, n->u.name, n->w, 0);
		break;

	case TNODE_ALT:
		render_alt(n, ctx);
		break;

	case TNODE_SEQ:
		render_seq(n, ctx);
		break;
	}
}

static void
render_rule(const struct tnode *node)
{
	struct render_context ctx;
	unsigned w;

	w = node->w + 8;

	ctx.paths = NULL;

	ctx.x = 0;
	ctx.y = node->a;
	printf("    <path d='M%u0 %u0 h%d0' marker-start='url(%s)'/>\n",
		ctx.x, ctx.y, 2, "#rrd:start");

	/* TODO: do want this pointing the other way around, to join with the adjacent path */
	ctx.x = w - 4;
	printf("    <path d='M%u0 %u0 h%d0' marker-start='url(%s)'/>\n",
		ctx.x, ctx.y, -2, "#rrd:start");

	ctx.x = 2;
	ctx.y = node->a;
	node_walk_render(node, &ctx);

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

static void
dim_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
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
		switch (*p) {
		case 'i': case 'I':
		case 'j':
		case 'l':
		case '1':
		case '|':
			n += 0.6;
			break;

		case 'm': case 'M':
		case 'w': case 'W':
			n += 1.3;
			break;

		default:
			n += 1.0;
			break;
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
svg_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;
	unsigned z;

	printf("<svg\n");
	printf("  xmlns='http://www.w3.org/2000/svg'\n");
	printf("  xmlns:xlink='http://www.w3.org/1999/xlink'\n");
	printf("\n");
	printf("  class='figure'\n");
	printf("  viewBox='-20 -50 900 2600'\n"); /* TODO */
	printf("  width='900' height='2600'>\n");
	printf("\n");

	printf("  <style>\n");
	printf("    rect, line, path { stroke-width: 1.5px; stroke: black; fill: transparent; }\n");
	printf("    path { fill: transparent; }\n");
	printf("    line.ellipsis { stroke-dasharray: 4; }\n");
	printf("    path.arrow.rtl { marker-mid: url(#rrd:arrow-rtl); }\n");
	printf("    path.arrow.ltr { marker-mid: url(#rrd:arrow-ltr); }\n");
	printf("  </style>\n");
	printf("\n");

	printf("  <defs>\n");
	printf("    <marker id='rrd:start'\n");
	printf("        markerWidth='10' markerHeight='12'\n");
	printf("        markerUnits='userSpaceOnUse'\n");
	printf("        refX='7' refY='6'\n");
	printf("        orient='auto'>\n"); /* TODO: auto-start-reverse in SVG2 */
	printf("      <line x1='7' y1='0' x2='7' y2='12' class='arrow'/>\n");
	printf("      <line x1='2' y1='0' x2='2' y2='12' class='arrow'/>\n");
	printf("    </marker>\n");
	printf("\n");

	printf("    <marker id='rrd:arrow-ltr'\n");
	printf("        markerWidth='5' markerHeight='5'\n");
	printf("        refX='3' refY='2.5'\n");
	printf("        orient='auto'>\n");
	printf("      <polyline points='0,0 5,2.5 0,5' class='arrow'/>\n");
	printf("    </marker>\n");
	printf("    <marker id='rrd:arrow-rtl'\n");
	printf("        markerWidth='5' markerHeight='5'\n");
	printf("        refX='3' refY='2.5'\n");
	printf("        orient='auto'>\n");
	printf("      <polyline points='5,0 0,2.5 5,5' class='arrow'/>\n");
	printf("    </marker>\n");
	printf("  </defs>\n");
	printf("\n");

	z = 0;

	for (p = grammar; p; p = p->next) {
		struct node *rrd;
		struct tnode *tnode;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		tnode = rrd_to_tnode(rrd, dim_string);

		node_free(rrd);

		printf("  <g transform='translate(%u0 %u0)'>\n",
			4, z);
		printf("    <text x='-%u0' y='-%u0'>%s:</text>\n",
			4, 2, p->name);

		render_rule(tnode);

		printf("  </g>\n");
		printf("\n");

z += tnode->a + tnode->d + 4; /* XXX */

		tnode_free(tnode);
	}

	printf("</svg>\n");
}

