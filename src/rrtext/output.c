/*
 * Copyright 2014-2019 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Railroad Diagram ASCII-Art Output
 *
 * Output a plaintext diagram of the abstract representation of railroads
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
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"

#include "io.h"
#include "tnode.h"

struct render_context {
	char **lines;
	char *scratch;
	int rtl;
	int x, y;
};

static void node_walk_render(const struct tnode *n, struct render_context *ctx);

/*
 * Trim trailing whitespace from a string. Whitespace is defined by isspace().
 *
 * Returns s, modified to be terminated at the start of its trailing whitespace.
 */
static char *
rtrim(char *s)
{
	char *p = s + strlen(s) - 1;

	while (p >= s && isspace((unsigned char) *p)) {
		*p = '\0';
		p--;
	}

	return s;
}

static int
bprintf(struct render_context *ctx, const char *fmt, ...)
{
	va_list ap;
	int n;

	assert(ctx != NULL);
	assert(ctx->scratch != NULL);

	va_start(ap, fmt);
	n = vsprintf(ctx->scratch, fmt, ap);
	va_end(ap);

	memcpy(ctx->lines[ctx->y] + ctx->x, ctx->scratch, n);

	return n;
}

/* XXX: static */
size_t
loop_label(const struct tnode *loop, char *s)
{
	char buffer[128];

	assert(loop->type == TNODE_LOOP);

	if (s == NULL) {
		s = buffer;
	}

	switch (loop->u.loop.looptype) {
	case TNODE_LOOP_ONCE:
		return sprintf(s, "(exactly once)");

	case TNODE_LOOP_ATLEAST:
		return sprintf(s, "(at least %d times)", loop->u.loop.min);

	case TNODE_LOOP_UPTO:
		return sprintf(s, "(up to %d times)", loop->u.loop.max);

	case TNODE_LOOP_EXACTLY:
		return sprintf(s, "(%d times)", loop->u.loop.max);

	case TNODE_LOOP_BETWEEN:
		return sprintf(s, "(%d-%d times)", loop->u.loop.min, loop->u.loop.max);
	}

	return 0;
}

static void
segment(struct render_context *ctx, const struct tnode *n, int delim)
{
	int y = ctx->y;
	ctx->y -= n->y;
	node_walk_render(n, ctx);

	ctx->x += n->w;
	ctx->y = y;
	if (delim) {
		bprintf(ctx, "--");
		ctx->x += 2;
	}
}

static void
justify(struct render_context *ctx, const struct tnode *n, int space)
{
	int x = ctx->x;
	int off = (space - n->w) / 2;

	for (; ctx->x < x + off; ctx->x++) {
		bprintf(ctx, "-");
	}

	ctx->y -= n->y;
	node_walk_render(n, ctx);

	ctx->y += n->y;
	ctx->x += n->w;
	for (; ctx->x < x + space; ctx->x++) {
		bprintf(ctx, "-");
	}

	ctx->x = x;
}

static void
render_alt(const struct tnode *n, struct render_context *ctx)
{
	int x, y;
	int line;
	char *a_in, *a_out;
	size_t j;

	assert(n != NULL);
	assert(n->type == TNODE_ALT || n->type == TNODE_ALT_SKIPPABLE);
	assert(ctx != NULL);

	x = ctx->x;
	y = ctx->y;
	line = y + n->y;

	if (n->type == TNODE_ALT_SKIPPABLE) {
		a_in  = n->y ? "v" : "^";
		a_out = n->y ? "^" : "v";
	} else {
		a_in  = "^";
		a_out = "v";

		ctx->y += n->y;
	}

	if (n->type == TNODE_ALT_SKIPPABLE) {
		int i;

		/*
		 * TODO: decide whether to put the skip above or hang it below.
		 * It looks nicer below when the item being skipped is low in height,
		 * and where adjacent SEQ nodes do not themselves go above the line.
		 */

		ctx->x = x;

		if (!ctx->rtl) {
			bprintf(ctx, ">");
		} else {
			bprintf(ctx, "<");
		}
		ctx->x++;

		for (i = 0; i < n->w - 2; i++) {
			bprintf(ctx, "-");
			ctx->x++;
		}

		bprintf(ctx, a_in);
		ctx->y++;

		for (i = 0; i < n->y - 1; i++) {
			ctx->x = x;
			bprintf(ctx, "|");
			ctx->x = x + n->w - 1;
			bprintf(ctx, "|");
			ctx->y++;
		}
	}

	for (j = 0; j < n->u.alt.n; j++) {
		int i, flush = ctx->y == line;

		/*
		 * Skip nodes are rendered as three-way branches,
		 * so we use ">" and "<" for the entry point,
		 * depending on rtl.
		 */

		ctx->x = x;
		if (!ctx->rtl) {
			if (n->type == TNODE_ALT_SKIPPABLE && j + 1 < n->u.alt.n) {
				bprintf(ctx, "+");
			} else {
				bprintf(ctx, flush ? a_out : ">");
			}
		} else {
			bprintf(ctx, flush ? "<" : (n->type == TNODE_ALT_SKIPPABLE ? "^" : a_in));
		}

		ctx->x += 1;
		justify(ctx, n->u.alt.a[j], n->w - 2);

		ctx->x = x + n->w - 1;
		if (!ctx->rtl) {
			bprintf(ctx, flush ? ">" : (n->type == TNODE_ALT_SKIPPABLE ? "^" : a_in));
		} else {
			if (n->type == TNODE_ALT_SKIPPABLE && j + 1 < n->u.alt.n) {
				bprintf(ctx, "+");
			} else {
				bprintf(ctx, flush ? a_out : "<");
			}
		}
		ctx->y++;

		if (j + 1 < n->u.alt.n) {
			for (i = 0; i < n->u.alt.a[j]->h - n->u.alt.a[j]->y + n->u.alt.a[j + 1]->y; i++) {
				ctx->x = x;
				bprintf(ctx, "|");
				ctx->x = x + n->w - 1;
				bprintf(ctx, "|");
				ctx->y++;
			}
		}
	}

	ctx->x = x;
	ctx->y = y;
}

static void
render_seq(const struct tnode *n, struct render_context *ctx)
{
	int x, y;

	assert(n != NULL);
	assert(n->type == TNODE_SEQ);
	assert(ctx != NULL);

	x = ctx->x;
	y = ctx->y;

	ctx->y += n->y;
	if (!ctx->rtl) {
		size_t i;

		for (i = 0; i < n->u.seq.n; i++) {
			segment(ctx, n->u.seq.a[i], i + 1 < n->u.seq.n);
		}
	} else {
		size_t i;

		for (i = 0; i < n->u.seq.n; i++) {
			segment(ctx, n->u.seq.a[n->u.seq.n - i], i + 1 < n->u.seq.n);
		}
	}

	ctx->x = x;
	ctx->y = y;
}

static void
render_loop(const struct tnode *n, struct render_context *ctx)
{
	int x = ctx->x, y = ctx->y;
	int cw;

	assert(n != NULL);
	assert(n->type == TNODE_LOOP);
	assert(ctx != NULL);

	ctx->y += n->y;
	bprintf(ctx, !ctx->rtl ? ">" : "v");
	ctx->x += 1;

	justify(ctx, n->u.loop.forward, n->w - 2);
	ctx->x = x + n->w - 1;
	bprintf(ctx, !ctx->rtl ? "v" : "<");
	ctx->y++;

	for (i = 0; i < n->u.loop.forward->h - n->u.loop.forward->y + n->u.loop.backward->y; i++) {
		ctx->x = x;
		bprintf(ctx, "|");
		ctx->x = x + n->w - 1;
		bprintf(ctx, "|");
		ctx->y++;
	}

	ctx->x = x;
	bprintf(ctx, !ctx->rtl ? "^" : ">");
	ctx->x += 1;
	ctx->rtl = !ctx->rtl;

	cw = loop_label(n, NULL);

	justify(ctx, n->u.loop.backward, n->w - 2);

	if (cw > 0) {
		int y = ctx->y;
		char c;
		ctx->x = x + 1 + (n->w - cw - 2) / 2;
		if (n->u.loop.backward->type != TNODE_SKIP) {
			ctx->y += 2;
		}
		/* still less horrible than malloc() */
		c = ctx->lines[ctx->y][ctx->x + cw];
		loop_label(n, ctx->lines[ctx->y] + ctx->x);
		ctx->lines[ctx->y][ctx->x + cw] = c;
		ctx->y = y;
	}

	ctx->rtl = !ctx->rtl;
	ctx->x = x + n->w - 1;
	bprintf(ctx, !ctx->rtl ? "<" : "^");

	ctx->x = x;
	ctx->y = y;
}

static void
node_walk_render(const struct tnode *n, struct render_context *ctx)
{
	assert(ctx != NULL);

	switch (n->type) {
	case TNODE_SKIP:
		break;

	case TNODE_LITERAL:
		bprintf(ctx, " \"%s\" ", n->u.literal);
		break;

	case TNODE_RULE:
		bprintf(ctx, " %s ", n->u.name);
		break;

	case TNODE_ALT:
	case TNODE_ALT_SKIPPABLE:
		render_alt(n, ctx);
		break;

	case TNODE_SEQ:
		render_seq(n, ctx);
		break;

	case TNODE_LOOP:
		render_loop(n, ctx);
		break;
	}
}

static void
render_rule(const struct tnode *node)
{
	struct render_context ctx;
	unsigned w, h;
	int i;

	w = node->w + 8;
	h = node->h;

	ctx.lines = xmalloc(sizeof *ctx.lines * h + 1);
	for (i = 0; i < h; i++) {
		ctx.lines[i] = xmalloc(w + 1);
		memset(ctx.lines[i], ' ', w);
		ctx.lines[i][w] = '\0';
	}

	ctx.rtl = 0;
	ctx.x = 0;
	ctx.y = 0;
	ctx.scratch = xmalloc(w + 1);

	ctx.y = node->y;
	bprintf(&ctx, "||--");

	ctx.x = w - 4;
	bprintf(&ctx, "--||");

	ctx.x = 4;
	ctx.y = 0;
	node_walk_render(node, &ctx);

	for (i = 0; i < h; i++) {
		rtrim(ctx.lines[i]);
		printf("    %s\n", ctx.lines[i]);
		free(ctx.lines[i]);
	}

	free(ctx.lines);
	free(ctx.scratch);
}

void
rrtext_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

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

		tnode = rrd_to_tnode(rrd);

		printf("%s:\n", p->name);
		render_rule(tnode);
		printf("\n");

		node_free(rrd); /* XXX: move earlier, when dim_*() are in struct tnode */

		tnode_free(tnode);
	}
}

