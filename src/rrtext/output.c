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

static void
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

	ctx->x += n;
}

static void
justify(struct render_context *ctx, const struct tnode *n, int space)
{
	unsigned lhs = (space - n->w) / 2;
	unsigned rhs = (space - n->w) - lhs;
	unsigned i;

	for (i = 0; i < lhs; i++) {
		bprintf(ctx, n->type == TNODE_ELLIPSIS ? " " : "-");
	}

	node_walk_render(n, ctx);

	for (i = 0; i < rhs; i++) {
		bprintf(ctx, n->type == TNODE_ELLIPSIS ? " " : "-");
	}
}

static void
bars(struct render_context *ctx, unsigned n, unsigned w)
{
	unsigned i;
	unsigned x;

	x = ctx->x;

	for (i = 0; i < n; i++) {
		bprintf(ctx, "|");
		ctx->x += w - 2;
		bprintf(ctx, "|");
		ctx->y++;
		ctx->x = x;
	}
}

static void
render_alt(const struct tnode *n, struct render_context *ctx)
{
	int x, y;
	size_t j;

	assert(n != NULL);
	assert(n->type == TNODE_ALT);
	assert(ctx != NULL);

	x = ctx->x;
	y = ctx->y;

	/*
	 * n->o is in terms of indicies; here we would iterate and add the y-distance
	 * for each of those nodes. That'd be .a for the topmost node, plus the height
	 * of each subsequent node and the depth of each node above it.
	 */
	assert(n->o <= 1); /* currently only implemented for one node above the line */
	if (n->o == 1) {
		ctx->y -= n->a;
	}

	for (j = 0; j < n->u.alt.n; j++) {
		const char *a;

		ctx->x = x;

		switch (n->u.alt.b[j]) {
		case TLINE_A: a = n->rtl ? "<^" : "^>"; break;
		case TLINE_B: a = ",.";                 break;
		case TLINE_C: a = n->rtl ? "<v" : "v>"; break;
		case TLINE_D: a = n->rtl ? "<+" : "+>"; break;
		case TLINE_E: a = "`'";                 break;
		case TLINE_F: a = "||";                 break;
		case TLINE_G: a = n->rtl ? "^<" : ">^"; break;
		case TLINE_H: a = n->rtl ? "v<" : ">v"; break;

		default:
			a = "??";
			break;
		}

		bprintf(ctx, "%c", a[0]);
		justify(ctx, n->u.alt.a[j], n->w - 2);
		bprintf(ctx, "%c", a[1]);

		if (j + 1 < n->u.alt.n) {
			ctx->y++;
			ctx->x = x;
			bars(ctx, n->u.alt.a[j]->d + n->u.alt.a[j + 1]->a, n->w);
		}
	}

	ctx->y = y;
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
			bprintf(ctx, "--");
		}
	}
}

static void
node_walk_render(const struct tnode *n, struct render_context *ctx)
{
	assert(ctx != NULL);

	switch (n->type) {
	case TNODE_SKIP:
		bprintf(ctx, "%.*s", (int) n->w, n->rtl ? "<" : ">");
		break;

	case TNODE_ELLIPSIS:
		bprintf(ctx, ":");
		break;

	case TNODE_LITERAL:
		bprintf(ctx, " \"%s\" ", n->u.literal);
		break;

	case TNODE_LABEL:
		bprintf(ctx, "%s", n->u.label);
		break;

	case TNODE_RULE:
		bprintf(ctx, " %s ", n->u.name);
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
	unsigned w, h;
	int i;

	w = node->w + 8;
	h = node->a + node->d;

	ctx.lines = xmalloc(sizeof *ctx.lines * h + 1);
	for (i = 0; i < h; i++) {
		ctx.lines[i] = xmalloc(w + 1);
		memset(ctx.lines[i], ' ', w);
		ctx.lines[i][w] = '\0';
	}

	ctx.x = 0;
	ctx.y = 0;
	ctx.scratch = xmalloc(w + 1);

	ctx.y = node->a;
	bprintf(&ctx, "||--");

	ctx.x = w - 4;
	bprintf(&ctx, "--||");

	ctx.x = 4;
	ctx.y = node->a;
	node_walk_render(node, &ctx);

	for (i = 0; i < h; i++) {
		rtrim(ctx.lines[i]);
		printf("    %s\n", ctx.lines[i]);
		free(ctx.lines[i]);
	}

	free(ctx.lines);
	free(ctx.scratch);
}

static void
dim_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	*w = strlen(s); /* monospace */
	*a = 0;
	*d = 1;
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

		tnode = rrd_to_tnode(rrd, dim_string);

		node_free(rrd);

		printf("%s:\n", p->name);
		render_rule(tnode);
		printf("\n");

		tnode_free(tnode);
	}
}

