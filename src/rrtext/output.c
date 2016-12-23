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
#include "../rrd/list.h"
#include "../rrd/stack.h"

#include "io.h"

struct render_context {
	struct box_size size;
	char **lines;
	char *scratch;
	int rtl;
	int x, y;
};

static void node_walk_render(struct node **n, struct render_context *ctx);

static int
bprintf(char *scratch, char *p, const char *fmt, ...)
{
	va_list ap;
	int n;

	assert(scratch != NULL);
	assert(p != NULL);

	va_start(ap, fmt);
	n = vsprintf(scratch, fmt, ap);
	va_end(ap);

	memcpy(p, scratch, n);

	return n;
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

static void
node_walk_dim(struct node **n)
{
	assert(n != NULL);

	switch ((*n)->type) {
		struct list **p;

	case NODE_SKIP:
		(*n)->size.w = 0;
		(*n)->size.h = 1;
		(*n)->y = 0;

		break;

	case NODE_LITERAL:
		(*n)->size.w = strlen((*n)->u.literal) + 4;
		(*n)->size.h = 1;
		(*n)->y = 0;

		break;

	case NODE_RULE:
		(*n)->size.w = strlen((*n)->u.name) + 2;
		(*n)->size.h = 1;
		(*n)->y = 0;

		break;

	case NODE_ALT:
		for (p = &(*n)->u.alt; *p != NULL; p = &(**p).next) {
			node_walk_dim(&(*p)->node);
		}

		{
			int w = 0, h = -1;
			struct list *q;

			for (q = (*n)->u.alt; q != NULL; q = q->next) {
				h += 1 + q->node->size.h;

				if (q->node->size.w > w) {
					w = q->node->size.w;
				}

				if (q == (*n)->u.alt) {
					if (q->node->type == NODE_SKIP && q->next && !q->next->next) {
						(*n)->y = 2 + q->node->y + q->next->node->y;
					} else {
						(*n)->y = q->node->y;
					}
				}
			}

			(*n)->size.w = w + 6;
			(*n)->size.h = h;
		}

		break;

	case NODE_SEQ:
		for (p = &(*n)->u.seq; *p != NULL; p = &(**p).next) {
			node_walk_dim(&(*p)->node);
		}

		{
			int w = 0, top = 0, bot = 1;
			struct list *q;

			for (q = (*n)->u.seq; q != NULL; q = q->next) {
				w += q->node->size.w + 2;
				if (q->node->y > top) {
					top = q->node->y;
				}
				if (q->node->size.h - q->node->y > bot) {
					bot = q->node->size.h - q->node->y;
				}
			}

			(*n)->size.w = w - 2;
			(*n)->size.h = bot + top;
			(*n)->y = top;
		}

		break;

	case NODE_LOOP:
		node_walk_dim(&(*n)->u.loop.forward);
		node_walk_dim(&(*n)->u.loop.backward);

		{
			int wf, wb, cw;

			wf = (*n)->u.loop.forward->size.w;
			wb = (*n)->u.loop.backward->size.w;

			(*n)->size.w = (wf > wb ? wf : wb) + 6;
			(*n)->size.h = (*n)->u.loop.forward->size.h + (*n)->u.loop.backward->size.h + 1;
			(*n)->y = (*n)->u.loop.forward->y;

			cw = loop_label(*n, NULL);

			if (cw > 0) {
				if (cw + 6 > (*n)->size.w) {
					(*n)->size.w = cw + 6;
				}
				if ((*n)->u.loop.backward->type != NODE_SKIP) {
					(*n)->size.h += 2;
				}
			}
		}

		break;
	}
}

static void
segment(struct render_context *ctx, struct node *n, int delim)
{
	int y = ctx->y;
	ctx->y -= n->y;
	node_walk_render(&n, ctx);

	ctx->x += n->size.w;
	ctx->y = y;
	if (delim) {
		bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "--");
		ctx->x += 2;
	}
}

static void
justify(struct render_context *ctx, struct node *n, int space)
{
	int x = ctx->x;
	int off = (space - n->size.w) / 2;

	for (; ctx->x < x + off; ctx->x++) {
		bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "-");
	}

	ctx->y -= n->y;
	node_walk_render(&n, ctx);

	ctx->y += n->y;
	ctx->x += n->size.w;
	for (; ctx->x < x + space; ctx->x++) {
		bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "-");
	}

	ctx->x = x;
}

static void
node_walk_render(struct node **n, struct render_context *ctx)
{
	assert(n != NULL);

	assert(ctx != NULL);

	switch ((*n)->type) {
	case NODE_LITERAL:
		bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, " \"%s\" ", (*n)->u.literal);

		break;

	case NODE_RULE:
		bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, " %s ", (*n)->u.name);

		break;

	case NODE_ALT:
		{
			struct list *p;
			int x, y;
			int line;
			char *a_in, *a_out;

			x = ctx->x;
			y = ctx->y;
			line = y + (*n)->y;

			/* XXX: suspicious. is (*n)->u.alt->node always present? */
			a_in  = ((*n)->y - (*n)->u.alt->node->y) ? "v" : "^";
			a_out = ((*n)->y - (*n)->u.alt->node->y) ? "^" : "v";

			ctx->y += (*n)->u.alt->node->y;

			for (p = (*n)->u.alt; p != NULL; p = p->next) {
				int i, flush = ctx->y == line;

				ctx->x = x;
				if (!ctx->rtl) {
					bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, flush ? a_out : ">");
				} else {
					bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, flush ? "<" : a_in);
				}

				ctx->x += 1;
				justify(ctx, p->node, (*n)->size.w - 2);

				ctx->x = x + (*n)->size.w - 1;
				if (!ctx->rtl) {
					bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, flush ? ">" : a_in);
				} else {
					bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, flush ? a_out : "<");
				}
				ctx->y++;

				if (p->next) {
					for (i = 0; i < p->node->size.h - p->node->y + p->next->node->y; i++) {
						ctx->x = x;
						bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "|");
						ctx->x = x + (*n)->size.w - 1;
						bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "|");
						ctx->y++;
					}
				}
			};

			ctx->x = x;
			ctx->y = y;
		}

		break;

	case NODE_SEQ:
		{
			struct list *p;
			struct node *q;
			int x, y;

			x = ctx->x;
			y = ctx->y;

			ctx->y += (*n)->y;
			if (!ctx->rtl) {
				for (p = (*n)->u.seq; p != NULL; p = p->next) {
					segment(ctx, p->node, !!p->next);
				}
			} else {
				struct stack *rl;

				rl = NULL;

				for (p = (*n)->u.seq; p != NULL; p = p->next) {
					stack_push(&rl, p->node);
				}

				while (q = stack_pop(&rl), q != NULL) {
					segment(ctx, q, !!rl);
				}
			}

			ctx->x = x;
			ctx->y = y;
		}

		break;

	case NODE_LOOP:
		{
			int x = ctx->x, y = ctx->y;
			int i, cw;

			ctx->y += (*n)->y;
			bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, !ctx->rtl ? ">" : "v");
			ctx->x += 1;

			justify(ctx, (*n)->u.loop.forward, (*n)->size.w - 2);
			ctx->x = x + (*n)->size.w - 1;
			bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, !ctx->rtl ? "v" : "<");
			ctx->y++;

			for (i = 0; i < (*n)->u.loop.forward->size.h - (*n)->u.loop.forward->y + (*n)->u.loop.backward->y; i++) {
				ctx->x = x;
				bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "|");
				ctx->x = x + (*n)->size.w - 1;
				bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, "|");
				ctx->y++;
			}

			ctx->x = x;
			bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, !ctx->rtl ? "^" : ">");
			ctx->x += 1;
			ctx->rtl = !ctx->rtl;

			cw = loop_label(*n, NULL);

			justify(ctx, (*n)->u.loop.backward, (*n)->size.w - 2);

			if (cw > 0) {
				int y = ctx->y;
				char c;
				ctx->x = x + 1 + ((*n)->size.w - cw - 2) / 2;
				if ((*n)->u.loop.backward->type != NODE_SKIP) {
					ctx->y += 2;
				}
				/* still less horrible than malloc() */
				c = ctx->lines[ctx->y][ctx->x + cw];
				loop_label(*n, ctx->lines[ctx->y] + ctx->x);
				ctx->lines[ctx->y][ctx->x + cw] = c;
				ctx->y = y;
			}

			ctx->rtl = !ctx->rtl;
			ctx->x = x + (*n)->size.w - 1;
			bprintf(ctx->scratch, ctx->lines[ctx->y] + ctx->x, !ctx->rtl ? "<" : "^");

			ctx->x = x;
			ctx->y = y;
		}

		break;

	case NODE_SKIP:
		break;
	}
}

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
			rrd_pretty_collapse(&rrd);
		}

		printf("%s:\n", p->name);

		{
			struct render_context ctx;
			int i;

			node_walk_dim(&rrd);

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
			bprintf(ctx.scratch, ctx.lines[ctx.y] + ctx.x, "||--");

			ctx.x = ctx.size.w - 4;
			bprintf(ctx.scratch, ctx.lines[ctx.y] + ctx.x, "--||");

			ctx.x = 4;
			ctx.y = 0;
			node_walk_render(&rrd, &ctx);

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

