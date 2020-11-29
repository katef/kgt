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
#include "../xalloc.h"
#include "../compiler_specific.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/node.h"
#include "../rrd/rrd.h"
#include "../rrd/list.h"
#include "../rrd/tnode.h"

#include "io.h"

struct render_context {
	char **lines;
	char *scratch;
	unsigned x, y; /* in character indicies */
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
	unsigned n;

	assert(ctx != NULL);
	assert(ctx->scratch != NULL);

	va_start(ap, fmt);
	n = vsprintf(ctx->scratch, fmt, ap);
	va_end(ap);

	memcpy(ctx->lines[ctx->y] + ctx->x, ctx->scratch, n);

	ctx->x += n;
}

/* made-up to suit text output */
static void
escputc(struct render_context *ctx, char c)
{
	assert(ctx != NULL);

	switch (c) {
	case '\\': bprintf(ctx, "\\\\"); return;
	case '\"': bprintf(ctx, "\\\""); return;

	case '\a': bprintf(ctx, "\\a"); return;
	case '\b': bprintf(ctx, "\\b"); return;
	case '\f': bprintf(ctx, "\\f"); return;
	case '\n': bprintf(ctx, "\\n"); return;
	case '\r': bprintf(ctx, "\\r"); return;
	case '\t': bprintf(ctx, "\\t"); return;
	case '\v': bprintf(ctx, "\\v"); return;

	default:
		break;
	}

	if (!isprint((unsigned char) c)) {
		bprintf(ctx, "\\x%02x", (unsigned char) c);
		return;
	}

	bprintf(ctx, "%c", c);
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
justify(struct render_context *ctx, const struct tnode *n, unsigned space)
{
	unsigned lhs, rhs;
	unsigned i;

	assert(n != NULL);
	assert(space >= n->w);

	centre(&lhs, &rhs, space, n->w);

	for (i = 0; i < lhs; i++) {
		bprintf(ctx, n->type == TNODE_ELLIPSIS ? " " : "\022");
	}

	node_walk_render(n, ctx);

	for (i = 0; i < rhs; i++) {
		bprintf(ctx, n->type == TNODE_ELLIPSIS ? " " : "\022");
	}
}

static void
bars(struct render_context *ctx, unsigned n, unsigned w)
{
	unsigned i;
	unsigned x;

	x = ctx->x;

	for (i = 0; i < n; i++) {
		bprintf(ctx, "\017");
		ctx->x += w - 2;
		bprintf(ctx, "\017");
		ctx->y++;
		ctx->x = x;
	}
}

static char *tile[][2] = {
	{ NULL, NULL }, /* \000 */
	{ "^", "\xe2\x95\xb0" }, /* \001 */
	{ ",", "\xe2\x95\xad" }, /* \002 */
	{ ".", "\xe2\x95\xae" }, /* \003 */
	{ "v", "\xe2\x95\xad" }, /* \004 */
	{ "+", "\xe2\x94\xbc" }, /* \005 */
	{ "`", "\xe2\x95\xb0" }, /* \006 */

	/* whitespace, not used because of rtrim() */
	{ NULL, NULL }, /* \007 */
	{ NULL, NULL }, /* \010 */
	{ NULL, NULL }, /* \011 */
	{ NULL, NULL }, /* \012 */
	{ NULL, NULL }, /* \013 */
	{ NULL, NULL }, /* \014 */
	{ NULL, NULL }, /* \015 */

	{ "'", "\xe2\x95\xaf" }, /* \016 */
	{ "|", "\xe2\x94\x82" }, /* \017 */
	{ ">", "\xe2\x95\xad" }, /* \020 */
	{ "<", "\xe2\x95\xaf" }, /* \021 */

	{ "-", "\xe2\x94\x80" }, /* \022 */
	{ "|", "\xe2\x94\x9c" }, /* \023 */
	{ "|", "\xe2\x94\xa4" }, /* \024 */
	{ ">", "\xe2\x95\xb0" }, /* \025 */
	{ "<", "\xe2\x95\xae" }, /* \026 */
	{ "<", "\xe2\x94\xbc" }, /* \027 */
	{ "v", "\xe2\x95\xae" }, /* \030 */
	{ ">", "\xe2\x94\xbc" }, /* \031 */
	{ "^", "\xe2\x95\xaf" }  /* \032 */
};

static void
tile_puts(const char *s, int utf8)
{
	const char *p;

	for (p = s; *p != '\0'; p++) {
		if ((unsigned char) *p < sizeof tile / sizeof *tile) {
			printf("%s", tile[(unsigned char) *p][utf8]);
			continue;
		}

		printf("%c", *p);
	}
}

static void
render_tline(struct render_context *ctx, enum tline tline, int rhs)
{
	const char *a;

	assert(ctx != NULL);

	switch (tline) {
	case TLINE_A: a = "\021\001"; break; case TLINE_a: a = "\032\025"; break;
	case TLINE_B: a = "\002\003"; break;
	case TLINE_C: a = "\026\004"; break; case TLINE_c: a = "\030\020"; break;
	case TLINE_D: a = "\027\005"; break; case TLINE_d: a = "\005\031"; break;
	case TLINE_E: a = "\006\016"; break;
	case TLINE_F: a = "\017\017"; break;
	case TLINE_G: a = "\001\021"; break; case TLINE_g: a = "\025\032"; break;
	case TLINE_H: a = "\004\026"; break; case TLINE_h: a = "\020\030"; break;
	case TLINE_I: a = "\001\021"; break; case TLINE_i: a = "\025\032"; break;
	case TLINE_J: a = "\022\022"; break;

	default:
		a = "??";
		break;
	}

	bprintf(ctx, "%c", a[rhs]);
}

static void
render_vlist(const struct tnode *n, struct render_context *ctx)
{
	unsigned x, y;
	size_t j;

	assert(n != NULL);
	assert(n->type == TNODE_VLIST);
	assert(ctx != NULL);

	x = ctx->x;
	y = ctx->y;

	/*
	 * .o is in terms of indicies; here we would iterate and add the y-distance
	 * for each of those nodes. That'd be .a for the topmost node, plus the height
	 * of each subsequent node and the depth of each node above it.
	 */
	assert(n->u.vlist.o <= 1); /* currently only implemented for one node above the line */
	if (n->u.vlist.o == 1) {
		ctx->y -= n->a;
	}

	/*
	 * A vlist of 0 items is a special case, meaning to draw
	 * a horizontal line only.
	 */
	if (n->u.vlist.n == 0) {
		size_t i;

		for (i = 0; i < n->w; i++) {
			bprintf(ctx, "\022");
		}
	} else for (j = 0; j < n->u.vlist.n; j++) {
		ctx->x = x;

		render_tline(ctx, n->u.vlist.b[j], 0);
		justify(ctx, n->u.vlist.a[j], n->w - 2);
		render_tline(ctx, n->u.vlist.b[j], 1);

		if (j + 1 < n->u.vlist.n) {
			ctx->y++;
			ctx->x = x;
			bars(ctx, n->u.vlist.a[j]->d + n->u.vlist.a[j + 1]->a, n->w);
		}
	}

	ctx->y = y;
}

static void
render_hlist(const struct tnode *n, struct render_context *ctx)
{
	size_t i;

	assert(n != NULL);
	assert(n->type == TNODE_HLIST);
	assert(ctx != NULL);

	for (i = 0; i < n->u.hlist.n; i++) {
		node_walk_render(n->u.hlist.a[i], ctx);

		if (i + 1 < n->u.hlist.n) {
			bprintf(ctx, "\022\022");
		}
	}
}

static void
render_comment(const struct tnode *n, struct render_context *ctx)
{
	unsigned x, y;
	unsigned lhs, rhs;

	assert(n != NULL);
	assert(n->type == TNODE_COMMENT);
	assert(ctx != NULL);

	x = ctx->x;
	y = ctx->y;

	justify(ctx, n->u.comment.tnode, n->w);

	assert(strlen(n->u.comment.s) <= n->w);
	centre(&lhs, &rhs, n->w, strlen(n->u.comment.s));

	ctx->x = x + lhs;
	ctx->y = y + n->d - 1;

	bprintf(ctx, "%s", n->u.comment.s);

	ctx->x += rhs;
	ctx->y = y;

	assert(ctx->x == x + n->w);
}

static void
render_txt(struct render_context *ctx, const struct txt *t)
{
	size_t i;

	assert(t != NULL);
	assert(t->p != NULL);

	for (i = 0; i < t->n; i++) {
		escputc(ctx, t->p[i]);
	}
}

static void
render_string(struct render_context *ctx, const char *s)
{
	struct txt t;

	t.p = s;
	t.n = strlen(s);

	render_txt(ctx, &t);
}

static void
node_walk_render(const struct tnode *n, struct render_context *ctx)
{
	assert(ctx != NULL);

	switch (n->type) {
	case TNODE_RTL_ARROW:
		bprintf(ctx, "%.*s", (int) n->w, "<");
		break;

	case TNODE_LTR_ARROW:
		bprintf(ctx, "%.*s", (int) n->w, ">");
		break;

	case TNODE_ELLIPSIS:
		bprintf(ctx, ":");
		break;

	case TNODE_CI_LITERAL:
		bprintf(ctx, " \"");
		render_txt(ctx, &n->u.literal);
		bprintf(ctx, "\"/i ");
		break;

	case TNODE_CS_LITERAL:
		bprintf(ctx, " \"");
		render_txt(ctx, &n->u.literal);
		bprintf(ctx, "\" ");
		break;

	case TNODE_PROSE:
		bprintf(ctx, "? ");
		render_string(ctx, n->u.prose);
		bprintf(ctx, " ?");
		break;

	case TNODE_COMMENT:
		render_comment(n, ctx);
		break;

	case TNODE_RULE:
		bprintf(ctx, " ");
		render_string(ctx, n->u.name);
		bprintf(ctx, " ");
		break;

	case TNODE_VLIST:
		render_vlist(n, ctx);
		break;

	case TNODE_HLIST:
		render_hlist(n, ctx);
		break;
	}
}

static void
render_rule(const struct tnode *node, int utf8)
{
	struct render_context ctx;
	unsigned w, h;
	unsigned i;

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
	bprintf(&ctx, "\017\023\022\022");

	ctx.x = w - 4;
	bprintf(&ctx, "\022\022\024\017");

	ctx.x = 4;
	ctx.y = node->a;
	node_walk_render(node, &ctx);

	for (i = 0; i < h; i++) {
		rtrim(ctx.lines[i]);
		printf("    ");
		tile_puts(ctx.lines[i], utf8);
		printf("\n");
		free(ctx.lines[i]);
	}

	free(ctx.lines);
	free(ctx.scratch);
}

static void
dim_mono_txt(const struct txt *t, unsigned *w, unsigned *a, unsigned *d)
{
	size_t i;
	unsigned n;

	assert(t != NULL);
	assert(t->p != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	n = 0;

	for (i = 0; i < t->n; i++) {
		switch (t->p[i]) {
		case '\\':
		case '\"':
		case '\a':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			n += 2;
			continue;

		default:
			break;
		}

		if (!isprint((unsigned char) t->p[i])) {
			n += 4;
			continue;
		}

		n += 1;
	}

	*w = n + 2;
	*a = 0;
	*d = 1;
}

static void
dim_mono_string(const char *s, unsigned *w, unsigned *a, unsigned *d)
{
	struct txt t;

	assert(s != NULL);
	assert(w != NULL);
	assert(a != NULL);
	assert(d != NULL);

	t.p = s;
	t.n = strlen(s);

	dim_mono_txt(&t, w, a, d);
}

void
rr_output(const struct ast_rule *grammar, struct dim *dim, int utf8)
{
	const struct ast_rule *p;

	assert(dim != NULL);

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

		tnode = rrd_to_tnode(rrd, dim);

		node_free(rrd);

		printf("%s:\n", p->name);
		render_rule(tnode, utf8);
		printf("\n");

		tnode_free(tnode);
	}
}

WARN_UNUSED_RESULT
int
rrutf8_output(const struct ast_rule *grammar)
{
	struct dim dim = {
		dim_mono_txt,
		dim_mono_string,
		2,
		0,
		2,
		0,
		2,
		1
	};

	rr_output(grammar, &dim, 1);
	return 1;
}

WARN_UNUSED_RESULT
int
rrtext_output(const struct ast_rule *grammar)
{
	struct dim dim = {
		dim_mono_txt,
		dim_mono_string,
		2,
		0,
		2,
		0,
		2,
		1
	};

	rr_output(grammar, &dim, 0);
	return 1;
}

