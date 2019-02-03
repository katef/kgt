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
#include <stdarg.h>
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

struct render_context {
	int x, y;
};

static void node_walk_render(const struct tnode *n, struct render_context *ctx);

static void
svg_text(struct render_context *ctx, const char *fmt, ...)
{
	va_list ap;

	assert(ctx != NULL);

	printf("    <text x='%u0' y='%u5' text-anchor='middle'>",
		ctx->x, ctx->y);

	/* TODO: escape characters */
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	printf("</text>\n");
}

static void
svg_rect(struct render_context *ctx, unsigned w, unsigned r)
{
	printf("    <rect x='%d0' y='%d0' height='%u0' width='%u0' rx='%u' ry='%u' stroke='black' fill='transparent'/>\n", /* XXX: css */
		(int) ctx->x, (int) ctx->y - 1,
		2, w,
		r, r);
}

static void
svg_textbox(struct render_context *ctx, const char *s, unsigned w, unsigned r)
{
	unsigned x;

	x = ctx->x;

	svg_rect(ctx, w, r);
	ctx->x += w / 2; /* XXX: either i want floats, or to scale things */
	svg_text(ctx, "%s", s);

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
svg_arrow(struct render_context *ctx, int w, int h,
	const char *marker_start, const char *marker_mid, const char *marker_end)
{
	printf("    <line x1='%u0' y1='%u0' x2='%d0' y2='%u0'",
		ctx->x, ctx->y,
		(int) ctx->x + w, ctx->y + h);

	if (marker_start != NULL) { printf(" marker-start='url(%s)'", marker_start); }
	if (marker_mid   != NULL) { printf(" marker-mid='url(%s)'",   marker_mid);   }
	if (marker_end   != NULL) { printf(" marker-end='url(%s)'",   marker_end);   }

	printf("/>\n");

	ctx->x += w;
}

static void
svg_line(struct render_context *ctx, int w, int h)
{
	svg_arrow(ctx, w, h, NULL, NULL, NULL);
	/* TODO: add to list, then combine adjoining lines */
}

static void
justify(struct render_context *ctx, const struct tnode *n, int space)
{
	unsigned lhs = (space - n->w) / 2;
	unsigned rhs = (space - n->w) - lhs;

	if (n->type != TNODE_ELLIPSIS) {
		svg_line(ctx, lhs, 0);
	} else {
		ctx->x += lhs;
	}

	node_walk_render(n, ctx);

	if (n->type != TNODE_ELLIPSIS) {
		svg_line(ctx, rhs, 0);
	} else {
		ctx->x += rhs;
	}

	ctx->y++;
}

static void
bars(struct render_context *ctx, unsigned n, unsigned w)
{
	svg_line(ctx, 0, n);
	ctx->x += w;
	svg_line(ctx, 0, n);
}

enum corner {
	/* entry from the lhs, exit on the rhs */
	CORNER_A, /* `- bottom left */
	CORNER_B, /* .- top left */
	CORNER_C, /* -' bottom right */
	CORNER_D  /* -. top right */
};

static void
render_corner(struct render_context *ctx, enum corner corner)
{
	int y;
	int rx, ry;

	switch (corner) {
	case CORNER_A: y =  1; rx = 0; ry = y; break;
	case CORNER_B: y = -1; rx = 0; ry = y; break;
	case CORNER_C: y = -1; rx = 1; ry = 0; break;
	case CORNER_D: y =  1; rx = 1; ry = 0; break;

	default:
		assert(!"unreached");
	}

	ctx->y += -y;

	printf("    <path d='M%u0 %u0 q %d0 %d0 %d0 %d0'/>\n",
		ctx->x, ctx->y, rx, ry, 1, y);

	ctx->x += 1;
	ctx->y -= -y;
}

static void
render_tline(struct render_context *ctx, enum tline tline, int rhs, int rtl)
{
	const char *a;

	assert(ctx != NULL);

	switch (tline) {
	case TLINE_A: a = rtl ? "AB" : "ba"; break;
	case TLINE_B: a = ",.";              break;
	case TLINE_C: a = rtl ? "CD" : "dc"; break;
	case TLINE_D: a = rtl ? "EF" : "fe"; break;
	case TLINE_E: a = "`'";              break;
	case TLINE_F: a = "||";              break;
	case TLINE_G: a = rtl ? "GH" : "hg"; break;
	case TLINE_H: a = rtl ? "IJ" : "ji"; break;
	case TLINE_I: a = rtl ? "KL" : "lk"; break;

	default:
		a = "??";
		break;
	}

	/* XXX: cheesy */
	switch (a[rhs]) {
	case ',': /* / top left */
		render_corner(ctx, CORNER_B);
		break;

	case '.': /* \ top right */
		render_corner(ctx, CORNER_D);
		break;

	case '`': /* \ bottom left */
		render_corner(ctx, CORNER_A);
		break;

	case '\'': /* / bottom right */
		ctx->y -= 2;
		render_corner(ctx, CORNER_C);
		break;

	case 'h': /* entry from left and top */
		render_corner(ctx, CORNER_A); /* exit right */
		break;

	case 'g': /* entry from left */
		ctx->y -= 2;
		render_corner(ctx, CORNER_C); /* exit up */
		ctx->y += 2;
		break;

	case 'a': /* entry from left and top */
		ctx->y -= 1;
		svg_line(ctx, 1, 0); /* entry from left, exit right */
		render_corner(ctx, CORNER_A); /* entry from top, exit right */
		ctx->x -= 1;
		ctx->y += 1;
		break;

	case 'd': /* entry from left */
		svg_line(ctx, 1, 0); /* exit right */
		ctx->x -= 2;
		ctx->y += 1;
		render_corner(ctx, CORNER_D); /* exit down */
		ctx->y -= 1;
		ctx->x += 1;
		break;

	case 'c': /* entry from left and bottom */
		ctx->y -= 1;
		svg_line(ctx, 1, 0); /* entry from left, exit right */
		render_corner(ctx, CORNER_B); /* entry from bottom, exit right */
		ctx->x -= 1;
		ctx->y += 1;
		break;

	case 'b': /* entry from left */
		ctx->x -= 1;
		ctx->y -= 1;
		render_corner(ctx, CORNER_C); /* exit up */
		ctx->y += 1;
		svg_line(ctx, 1, 0); /* exit right */
		break;

	case 'j': /* entry from left and bottom */
		svg_line(ctx, 1, 0); /* entry from left, exit right */
		ctx->x -= 1;
		render_corner(ctx, CORNER_B); /* entry from bottom, exit right */
		break;

	case 'i': /* entry from left */
		render_corner(ctx, CORNER_D); /* exit down */
		ctx->y -= 1;
		ctx->x -= 1;
		svg_line(ctx, 1, 0); /* exit right */
		ctx->y += 1;
		break;

	case 'l': /* entry from left and top */
		svg_line(ctx, 1, 0); /* entry from left, exit right */
		ctx->x -= 1;
		render_corner(ctx, CORNER_A); /* entry from top, exit right */
		break;

	case 'k': /* entry from left */
		ctx->y -= 1;
		svg_line(ctx, 1, 0); /* exit right */
		ctx->x -= 1;
		ctx->y -= 1;
		render_corner(ctx, CORNER_C); /* exit up */
		ctx->y += 2;
		break;

	case '|': /* nothing to draw */
		ctx->x += 1;
		break;

	default:
		svg_text(ctx, "%c", a[rhs]);
		ctx->x += 1;
		break;
	}
}

static void
render_alt(const struct tnode *n, struct render_context *ctx)
{
	int x, o, y;
	unsigned h;
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

		/* TODO: render entry/exit as a separate corner|line combination */
		/* TODO: decide this based on tline, not on n->o */
		if (j == n->o) {
			svg_line(ctx, 1, 0);
		} else {
			ctx->x += 1;
		}

		render_tline(ctx, n->u.alt.b[j], 0, n->rtl);
		justify(ctx, n->u.alt.a[j], n->w - 4);
		render_tline(ctx, n->u.alt.b[j], 1, n->rtl);

		if (j == n->o) {
			ctx->y -= 1;
			svg_line(ctx, 1, 0);
			ctx->y += 1;
		} else {
			ctx->x += 1;
		}

		if (j + 1 < n->u.alt.n) {
			ctx->y += n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a;
		}
	}

	h = 0;

	for (j = 0; j < n->u.alt.n; j++) {
		if (j + 1 < n->u.alt.n) {
			h += n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a + 1;
		}
	}

	ctx->x = x;
	ctx->y = y;

	h -= 2; /* for the tline corner pieces */
	ctx->y += 1;
/* XXX: need to render bars() in two separate parts, above and below the line */
ctx->x += 1;
	bars(ctx, h, n->w - 2);
ctx->x -= 1;

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
			svg_line(ctx, 2, 0);
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
		/* TODO: midpoint arrow */
		if (n->w > 0) {
			svg_text(ctx, "%s", n->rtl ? "&lt;" : "&gt;");
		}
		ctx->x += n->w;
		break;

	case TNODE_ELLIPSIS:
		/* TODO: 2 looks too long */
		svg_ellipsis(ctx, 0, 2);
		break;

	case TNODE_CI_LITERAL:
		/* TODO: render differently somehow, show /i suffix, maybe as a triangle in bottom-right corner */
		svg_textbox(ctx, n->u.literal, n->w, 0);
		svg_text(ctx, "/i");
		break;

	case TNODE_CS_LITERAL:
		/* TODO: square box, fill grey */
		svg_textbox(ctx, n->u.literal, n->w, 0);
		break;

	case TNODE_LABEL:
		/* TODO: no border, just text */
		svg_textbox(ctx, n->u.label, n->w, 8);
		break;

	case TNODE_RULE:
		/* TODO: rounded box */
		svg_textbox(ctx, n->u.name, n->w, 8);
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

	ctx.x = 0;
	ctx.y = 0;

	ctx.y = node->a;
	svg_arrow(&ctx,  2, 0, "#rrd:start", NULL, NULL);

	ctx.x = w - 4;
	svg_arrow(&ctx, -2, 0, "#rrd:start", NULL, NULL);

	ctx.x = 2;
	ctx.y = node->a;
	node_walk_render(node, &ctx);
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
	printf("    rect, line, path { stroke-width: 1.5px; stroke: black; }\n");
	printf("    path { fill: transparent; }\n");
	printf("    line.ellipsis { stroke-dasharray: 4; }\n");
	printf("  </style>\n");
	printf("\n");

	printf("  <defs>\n");
	printf("    <marker id='rrd:start'\n");
	printf("        markerWidth='10' markerHeight='12'\n");
	printf("        markerUnits='userSpaceOnUse'\n");
	printf("        refX='7' refY='6'\n");
	printf("        orient='auto'>\n");
	printf("      <line x1='7' y1='0' x2='7' y2='12' class='arrow'/>\n");
	printf("      <line x1='2' y1='0' x2='2' y2='12' class='arrow'/>\n");
	printf("    </marker>\n");
	printf("\n");
	printf("    <marker id='rrd:arrow'\n");
	printf("        markerWidth='8' markerHeight='8'\n");
	printf("        markerUnits='userSpaceOnUse'\n");
	printf("        refX='8' refY='4'\n");
	printf("        orient='auto'>\n");
	printf("      <polyline points='0,0 8,4 0,8' class='arrow'/>\n");
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

