/* $Id$ */

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

#include "../ast.h"
#include "../xalloc.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/stack.h"

#include "io.h"

struct render_context {
	struct box_size size;
	char **lines;
	char *scratch;
	int rtl;
	int x, y;
};

static void
bprintf(struct render_context *ctx, const char *fmt, ...)
{
	va_list ap;
	int s;
	char *d;

	va_start(ap, fmt);
	s = vsprintf(ctx->scratch, fmt, ap);
	va_end(ap);
	d = ctx->lines[ctx->y] + ctx->x;

	memcpy(d, ctx->scratch, s);
}

static struct node_walker w_dimension, w_render;

static int
dim_nothing(struct node *n, struct node **np, int depth, void *arg)
{
	(void) np;
	(void) arg;
	(void) depth;

	n->size.w = 0;
	n->size.h = 1;
	n->y = 0;

	return 1;
}

static int
dim_name(struct node *n, struct node **np, int depth, void *arg)
{
	(void) np;
	(void) arg;
	(void) depth;

	n->size.w = strlen(n->u.name) + 2;
	n->size.h = 1;
	n->y = 0;

	return 1;
}

static int
dim_literal(struct node *n, struct node **np, int depth, void *arg)
{
	(void) np;
	(void) arg;
	(void) depth;


	n->size.w = strlen(n->u.literal) + 4;
	n->size.h = 1;
	n->y = 0;

	return 1;
}

static int
dim_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	int w = 0, top = 0, bot = 1;
	struct node *p;

	(void) np;
	(void) arg;

	node_walk_list(&n->u.sequence, &w_dimension, depth + 1, arg);

	for (p = n->u.sequence; p != NULL; p = p->next) {
		w += p->size.w + 2;
		if (p->y > top) {
			top = p->y;
		}
		if (p->size.h - p->y > bot) {
			bot = p->size.h - p->y;
		}
	}

	n->size.w = w - 2;
	n->size.h = bot + top;
	n->y = top;

	return 1;
}

static int
dim_choice(struct node *n, struct node **np, int depth, void *arg)
{
	int w = 0, h = -1;
	struct node *p;

	(void) np;
	(void) arg;

	node_walk_list(&n->u.choice, &w_dimension, depth + 1, arg);

	for (p = n->u.choice; p != NULL; p = p->next) {
		h += 1 + p->size.h;

		if (p->size.w > w) {
			w = p->size.w;
		}

		if (p == n->u.choice) {
			if (p->type == NODE_SKIP && p->next && !p->next->next) {
				n->y = 2 + p->y + p->next->y;
			} else {
				n->y = p->y;
			}
		}
	}

	n->size.w = w + 6;
	n->size.h = h;

	return 1;
}

static size_t
loop_label(struct node *loop, char *s)
{
	char buffer[128];

	if (s == NULL) {
		s = buffer;
	}

	if (loop->u.loop.max == 1 && loop->u.loop.min == 1) {
		return sprintf(s, "(exactly once)");
	} else if (loop->u.loop.max == 0 && loop->u.loop.min > 0) {
		return sprintf(s, "(at least %d times)", loop->u.loop.min);
	} else if (loop->u.loop.max > 0 && loop->u.loop.min == 0) {
		return sprintf(s, "(up to %d times)", loop->u.loop.max);
	} else if (loop->u.loop.max > 0 && loop->u.loop.min == loop->u.loop.max) {
		return sprintf(s, "(%d times)", loop->u.loop.max);
	} else if (loop->u.loop.max > 1 && loop->u.loop.min > 1) {
		return sprintf(s, "(%d-%d times)", loop->u.loop.min, loop->u.loop.max);
	}

	return 0;
}

static int
dim_loop(struct node *n, struct node **np, int depth, void *arg)
{
	int wf, wb, cw;

	(void) np;
	(void) arg;

	node_walk(&n->u.loop.forward, &w_dimension, depth + 1, arg);
	wf = n->u.loop.forward->size.w;

	node_walk(&n->u.loop.backward, &w_dimension, depth + 1, arg);
	wb = n->u.loop.backward->size.w;

	n->size.w = (wf > wb ? wf : wb) + 6;
	n->size.h = n->u.loop.forward->size.h + n->u.loop.backward->size.h + 1;
	n->y = n->u.loop.forward->y;

	cw = loop_label(n, NULL);

	if (cw > 0) {
		if (cw + 6 > n->size.w) {
			n->size.w = cw + 6;
		}
		if (n->u.loop.backward->type != NODE_SKIP) {
			n->size.h += 2;
		}
	}

	return 1;
}

static struct node_walker w_dimension = {
	dim_nothing,
	dim_name, dim_literal,
	dim_choice,     dim_sequence,
	dim_loop
};

static int
render_literal(struct node *n, struct node **np, int depth, void *arg)
{
	struct render_context *ctx = arg;

	(void) np;
	(void) depth;

	bprintf(ctx, " \"%s\" ", n->u.literal);

	return 1;
}

static int
render_name(struct node *n, struct node **np, int depth, void *arg)
{
	struct render_context *ctx = arg;

	(void) np;
	(void) depth;

	bprintf(ctx, " %s ", n->u.name);

	return 1;
}

static void
segment(struct render_context *ctx, struct node *n, int depth, int delim)
{
	int y = ctx->y;
	ctx->y -= n->y;
	node_walk(&n, &w_render, depth, ctx);

	ctx->x += n->size.w;
	ctx->y = y;
	if (delim) {
		bprintf(ctx, "--");
		ctx->x += 2;
	}
}

static int
render_sequence(struct node *n, struct node **np, int depth, void *arg)
{
	/* ->-item1->-item2 */
	struct render_context *ctx = arg;
	struct node *p;
	int x = ctx->x, y = ctx->y;

	(void) np;

	ctx->y += n->y;
	if (!ctx->rtl) {
		for (p = n->u.sequence; p != NULL; p = p->next) {
			segment(ctx, p, depth + 1, !!p->next);
		}
	} else {
		struct stack *rl;

		rl = NULL;

		for (p = n->u.sequence; p != NULL; p = p->next) {
			stack_push(&rl, p);
		}

		while (p = stack_pop(&rl), p != NULL) {
			segment(ctx, p, depth + 1, !!rl);
		}
	}

	ctx->x = x;
	ctx->y = y;

	return 1;
}

static void
justify(struct render_context *ctx, int depth, struct node *n, int space)
{
	int x = ctx->x;
	int off = (space - n->size.w) / 2;

	for (; ctx->x < x + off; ctx->x++) {
		bprintf(ctx, "-");
	}

	ctx->y -= n->y;
	node_walk(&n, &w_render, depth, ctx);

	ctx->y += n->y;
	ctx->x += n->size.w;
	for (; ctx->x < x + space; ctx->x++) {
		bprintf(ctx, "-");
	}

	ctx->x = x;
}

static int
render_choice(struct node *n, struct node **np, int depth, void *arg)
{
	struct render_context *ctx = arg;
	struct node *p;
	int x = ctx->x, y = ctx->y;
	int line = y + n->y;
	char *a_in	= (n->y - n->u.choice->y) ? "v" : "^";
	char *a_out = (n->y - n->u.choice->y) ? "^" : "v";

	ctx->y += n->u.choice->y;

	(void) np;

	for (p = n->u.choice; p != NULL; p = p->next) {
		int i, flush = ctx->y == line;

		ctx->x = x;
		if (!ctx->rtl) {
			bprintf(ctx, flush ? a_out : ">");
		} else {
			bprintf(ctx, flush ? "<" : a_in);
		}

		ctx->x += 1;
		justify(ctx, depth + 1, p, n->size.w - 2);

		ctx->x = x + n->size.w - 1;
		if (!ctx->rtl) {
			bprintf(ctx, flush ? ">" : a_in);
		} else {
			bprintf(ctx, flush ? a_out : "<");
		}
		ctx->y++;

		if (p->next) {
			for (i = 0; i < p->size.h - p->y + p->next->y; i++) {
				ctx->x = x;
				bprintf(ctx, "|");
				ctx->x = x + n->size.w - 1;
				bprintf(ctx, "|");
				ctx->y++;
			}
		}
	};

	ctx->x = x;
	ctx->y = y;

	return 1;
}

static int
render_loop(struct node *n, struct node **np, int depth, void *arg)
{
	struct render_context *ctx = arg;
	int x = ctx->x, y = ctx->y;
	int i, cw;

	(void) np;

	ctx->y += n->y;
	bprintf(ctx, !ctx->rtl ? ">" : "v");
	ctx->x += 1;

	justify(ctx, depth + 1, n->u.loop.forward, n->size.w - 2);
	ctx->x = x + n->size.w - 1;
	bprintf(ctx, !ctx->rtl ? "v" : "<");
	ctx->y++;

	for (i = 0; i < n->u.loop.forward->size.h - n->u.loop.forward->y + n->u.loop.backward->y; i++) {
		ctx->x = x;
		bprintf(ctx, "|");
		ctx->x = x + n->size.w - 1;
		bprintf(ctx, "|");
		ctx->y++;
	}

	ctx->x = x;
	bprintf(ctx, !ctx->rtl ? "^" : ">");
	ctx->x += 1;
	ctx->rtl = !ctx->rtl;

	cw = loop_label(n, NULL);

	justify(ctx, depth + 1, n->u.loop.backward, n->size.w - 2);

	if (cw > 0) {
		int y = ctx->y;
		char c;
		ctx->x = x + 1 + (n->size.w - cw - 2) / 2;
		if (n->u.loop.backward->type != NODE_SKIP) {
			ctx->y += 2;
		}
		/* still less horrible than malloc() */
		c = ctx->lines[ctx->y][ctx->x + cw];
		loop_label(n, ctx->lines[ctx->y] + ctx->x);
		ctx->lines[ctx->y][ctx->x + cw] = c;
		ctx->y = y;
	}

	ctx->rtl = !ctx->rtl;
	ctx->x = x + n->size.w - 1;
	bprintf(ctx, !ctx->rtl ? "<" : "^");

	ctx->x = x;
	ctx->y = y;

	return 1;
}

static struct node_walker w_render = {
	0,
	render_name,   render_literal,
	render_choice, render_sequence,
	render_loop
};

void
rrtext_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p; p = p->next) {
		struct node *rrd;

		rrd = ast_to_rrd(p);
		if (rrd == NULL) {
			perror("ast_to_rrd");
			return;
		}

		if (prettify) {
			rrd_pretty_prefixes(&rrd);
			rrd_pretty_suffixes(&rrd);
			rrd_pretty_redundant(&rrd);
			rrd_pretty_bottom(&rrd);
		}

		printf("%s:\n", p->name);

		{
			struct render_context ctx;
			int i;

			node_walk(&rrd, &w_dimension, 0, 0);

			ctx.size = rrd->size;
			ctx.size.w += 8;

			ctx.lines = xmalloc(sizeof *ctx.lines * ctx.size.h + 1);
			for (i = 0; i < ctx.size.h; i++) {
				ctx.lines[i] = xmalloc(ctx.size.w + 1);
				memset(ctx.lines[i], ' ', ctx.size.w);
				ctx.lines[i][ctx.size.w] = '\0';
			}

			ctx.rtl = 0;
			ctx.x = 0;
			ctx.y = 0;
			ctx.scratch = xmalloc(ctx.size.w + 1);

			ctx.y = rrd->y;
			bprintf(&ctx, "||--");

			ctx.x = ctx.size.w - 4;
			bprintf(&ctx, "--||");

			ctx.x = 4;
			ctx.y = 0;
			node_walk(&rrd, &w_render, 0, &ctx);

			for (i = 0; i < ctx.size.h; i++) {
				printf("    %s\n", ctx.lines[i]);
				free(ctx.lines[i]);
			}

			free(ctx.lines);
			free(ctx.scratch);
		}

		printf("\n");

		node_free(rrd);
	}
}

