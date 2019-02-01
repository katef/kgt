/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#define _POSIX_C_SOURCE 2

#include <unistd.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "rewrite.h"
#include "xalloc.h"
#include "rrd/node.h"

#include "bnf/io.h"
#include "blab/io.h"
#include "wsn/io.h"
#include "abnf/io.h"
#include "iso-ebnf/io.h"
#include "rbnf/io.h"
#include "sid/io.h"
#include "dot/io.h"
#include "rrdot/io.h"
#include "rrdump/io.h"
#include "rrtdump/io.h"
#include "rrparcon/io.h"
#include "rrll/io.h"
#include "rrta/io.h"
#include "rrtext/io.h"

int prettify = 1;
int allow_undefined = 1;

struct io {
	const char *name;
	struct ast_rule *(*in)(int (*f)(void *), void *);
	void (*out)(const struct ast_rule *);
	enum ast_features ast_unsupported;
	enum rrd_features rrd_unsupported;
} io[] = {
	{ "bnf",      bnf_input,      bnf_output,      bnf_ast_unsupported, 0 },
	{ "blab",     NULL,           blab_output,     0, 0 },
	{ "wsn",      wsn_input,      wsn_output,      wsn_ast_unsupported, 0 },
	{ "abnf",     abnf_input,     NULL,            0, 0 },
	{ "iso-ebnf", iso_ebnf_input, iso_ebnf_output, iso_ebnf_ast_unsupported, 0 },
	{ "rbnf",     rbnf_input,     rbnf_output,     rbnf_ast_unsupported, 0 },
	{ "sid",      NULL,           sid_output,      sid_ast_unsupported, 0 },
	{ "dot",      NULL,           dot_output,      0, 0 },
	{ "rrdot",    NULL,           rrdot_output,    0, 0 },
	{ "rrdump",   NULL,           rrdump_output,   0, 0 },
	{ "rrtdump",  NULL,           rrtdump_output,  0, 0 },
	{ "rrparcon", NULL,           rrparcon_output, 0, rrparcon_rrd_unsupported },
	{ "rrll",     NULL,           rrll_output,     0, rrll_rrd_unsupported     },
	{ "rrta",     NULL,           rrta_output,     0, rrta_rrd_unsupported     },
	{ "rrtext",   NULL,           rrtext_output,   0, 0 }
};

enum io_dir {
	IO_IN,
	IO_OUT
};

static void
xusage(void)
{
	printf("usage: kgt [-nu] [-l <language>] [ -e <language> ]\n");
	exit(EXIT_FAILURE);
}

static int
kgt_fgetc(void *opaque)
{
	FILE *f;

	f = opaque;

	assert(f != NULL);

	return fgetc(f);
}

static struct io *
lang(enum io_dir dir, const char *s)
{
	size_t i;

	for (i = 0; i < sizeof io / sizeof *io; i++) {
		if (dir == IO_IN && io[i].in == NULL) {
			continue;
		}

		if (dir == IO_OUT && io[i].out == NULL) {
			continue;
		}

		if (0 == strcmp(s, io[i].name)) {
			return &io[i];
		}
	}

	fprintf(stderr, "Unrecognised %s language \"%s\"; supported languages are:",
		dir == IO_IN ? "input" : "output",
		s);

	for (i = 0; i < sizeof io / sizeof *io; i++) {
		if (dir == IO_IN && io[i].in == NULL) {
			continue;
		}

		if (dir == IO_OUT && io[i].out == NULL) {
			continue;
		}

		fprintf(stderr, " %s", io[i].name);
	}

	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct ast_rule *g;
	struct io *in, *out;

	in  = lang(IO_IN, "bnf");
	out = in;

	{
		int c;

		while ((c = getopt(argc, argv, "hnl:e:u")) != -1) {
			switch (c) {
			case 'l': in  = lang(IO_IN,  optarg); break;
			case 'e': out = lang(IO_OUT, optarg); break;

			case 'n':
				prettify = 0;
				break;

			case 'u':
				allow_undefined = 0;
				break;

			case '?':
			default:
				xusage();
			}
		}

		argc -= optind;
		argv += optind;

		if (argc > 0) {
			xusage();
		}
	}

	assert(io->in  != NULL);
	assert(io->out != NULL);

	g = in->in(kgt_fgetc, stdin);

	{
		unsigned v;

		for (v = out->ast_unsupported; v != 0; v &= v - 1) {
			int r;

			/* TODO: expose these rewritings as CLI options too; set as bits in v */
			/* TODO: option to query if output is possible without rewriting */
			switch (v & -v) {
			case FEATURE_AST_CI_LITERAL: r = 1; rewrite_ci_literals(g); break;
			}

			if (!r) {
				perror("ast_transform_*");
				exit(EXIT_FAILURE);
			}
		}
	}

	out->out(g);

	/* TODO: free ast */

	return EXIT_SUCCESS;
}

