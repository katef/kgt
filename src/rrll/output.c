/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * Lukas Lueg Railroad DSL.
 * See https://github.com/lukaslueg/railroad_dsl
 * and the library it uses for rendering: https://github.com/lukaslueg/railroad
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../txt.h"
#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/pretty.h"
#include "../rrd/rewrite.h"
#include "../rrd/node.h"
#include "../rrd/list.h"
#include "../compiler_specific.h"

#include "io.h"

/* unknown provenance */
static int
escputc(int c, FILE *f)
{
	size_t i;

	const struct {
		int c;
		const char *s;
	} a[] = {
		{ '\\', "\\\\" },
		{ '\"', "\\\"" },
		{ '/',  "\\/"  },

		{ '\b', "\\b"  },
		{ '\f', "\\f"  },
		{ '\n', "\\n"  },
		{ '\r', "\\r"  },
		{ '\t', "\\t"  }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (a[i].c == c) {
			return fputs(a[i].s, f);
		}
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "\\u%04x", (unsigned char) c);
	}

	return fprintf(f, "%c", c);
}

/* TODO: centralise, maybe with callback */
static int
escputs(const char *s, FILE *f)
{
	const char *p;
	int r, n;

	assert(s != NULL);
	assert(f != NULL);

	n = 0;

	for (p = s; *p != '\0'; p++) {
		r = escputc(*p, f);
		if (r < 0) {
			return -1;
		}

		n += r;
	}

	return n;
}

/* TODO: centralise */
static int
escputt(const struct txt *t, FILE *f)
{
	size_t i;
	int r;

	assert(t != NULL);
	assert(t->p != NULL);

	for (i = 0; i < t->n; i++) {
		r = escputc(t->p[i], f);
		if (r < 0) {
			return -1;
		}
	}

	return 0;
}

/* TODO: centralise */
static size_t
loop_label(unsigned min, unsigned max, char *s)
{
	assert(max >= min);
	assert(s != NULL);

	if (max == 1 && min == 1) {
		return sprintf(s, "(exactly once)");
	} else if (max == 0 && min > 0) {
		return sprintf(s, "(at least %u times)", min);
	} else if (max > 0 && min == 0) {
		return sprintf(s, "(up to %d times)", max);
	} else if (max > 0 && min == max) {
		return sprintf(s, "(%u times)", max);
	} else if (max > 1 && min > 1) {
		return sprintf(s, "(%u-%u times)", min, max);
	}

	*s = '\0';

	return 0;
}

static void
node_walk(FILE *f, const struct node *n)
{
	assert(f != NULL);

	if (n == NULL) {
		fprintf(f, "!");

		return;
	}

	assert(!n->invisible);

	switch (n->type) {
		const struct list *p;

	case NODE_CI_LITERAL:
		fprintf(stderr, "unimplemented\n");
		exit(EXIT_FAILURE);

	case NODE_CS_LITERAL:
		fprintf(f, "\"");
		escputt(&n->u.literal, f);
		fprintf(f, "\"");

		break;

	case NODE_RULE:
		fprintf(f, "'");
		escputs(n->u.name, f);
		fprintf(f, "'");

		break;

	case NODE_PROSE:
		/* TODO: escaping to somehow avoid ` */
		fprintf(f, "`");
		escputs(n->u.prose, f);
		fprintf(f, "`");

		break;

	case NODE_ALT:
	case NODE_ALT_SKIPPABLE:
		fprintf(f, "<");

		for (p = n->u.alt; p != NULL; p = p->next) {
			node_walk(f, p->node);
			if (p->next != NULL) {
				fprintf(f, ", ");
			}
		}
		fprintf(f, ">");

		if (n->type == NODE_ALT_SKIPPABLE) {
			fprintf(f, "?");
		}

		break;

	case NODE_SEQ:
		fprintf(f, "[");
		for (p = n->u.seq; p != NULL; p = p->next) {
			node_walk(f, p->node);
			if (p->next != NULL) {
				fprintf(f, " ");
			}
		}
		fprintf(f, "]");

		break;

	case NODE_LOOP:
		{
			char s[128]; /* XXX */
			size_t r;

			r = loop_label(n->u.loop.min, n->u.loop.max, s);

			node_walk(f, n->u.loop.forward);
			fprintf(f, "*");
			if (r > 0) {
				fprintf(f, "[`%s`", s);
			}
			node_walk(f, n->u.loop.backward);
			if (r > 0) {
				fprintf(f, "]");
			}
		}

		break;
	}
}

WARN_UNUSED_RESULT
int
rrll_output(const struct ast_rule *grammar)
{
	const struct ast_rule *p;

	for (p = grammar; p != NULL; p = p->next) {
		struct node *rrd;

		if (!ast_to_rrd(p, &rrd)) {
			perror("ast_to_rrd");
			return 0;
		}

		if (prettify) {
			rrd_pretty(&rrd);
		}

		/* TODO: pass in unsupported bitmap */
		if (!rewrite_rrd_ci_literals(rrd))
			return 0;

		printf("[`");
		escputs(p->name, stdout);
		printf("` ");

		node_walk(stdout, rrd);
		printf("]\n");

		node_free(rrd);
	}
	return 1;
}

