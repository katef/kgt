/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 500

#include <unistd.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "txt.h"
#include "ast.h"
#include "parsing_error.h"
#include "rewrite.h"
#include "xalloc.h"
#include "rrd/node.h"

#include "bnf/io.h"
#include "blab/io.h"
#include "ebnfhtml5/io.h"
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
#include "svg/io.h"
#include "html5/io.h"
#include "json/io.h"

int debug = 0;
int prettify = 1;
int allow_undefined = 1;
const char *css_file;

/*
 * emscripten's musl will call __wait from getopt() (via getopt_msg and
 * then flockfile), which is not present for standalone wasm because that
 * has no pthread support, and so a definition for __wait is not included.
 */
#ifdef __EMSCRIPTEN__
void __wait(int a, int b, int c, int d) { }
#endif

struct io {
	const char *name;
	struct ast_rule *(*in)(int (*f)(void *), void *, parsing_error_queue*);
	int (*out)(const struct ast_rule *);
	enum ast_features ast_unsupported;
	enum rrd_features rrd_unsupported;
} io[] = {
	{ "bnf",        bnf_input,      bnf_output,         bnf_ast_unsupported, 0 },
	{ "blab",       NULL,           blab_output,        blab_ast_unsupported, 0 },
	{ "ebnfhtml5",  NULL,           ebnf_html5_output,  ebnf_html5_ast_unsupported, 0 },
	{ "ebnfxhtml5", NULL,           ebnf_xhtml5_output, ebnf_html5_ast_unsupported, 0 },
	{ "wsn",        wsn_input,      wsn_output,         wsn_ast_unsupported, 0 },
	{ "abnf",       abnf_input,     abnf_output,        0, 0 },
	{ "iso-ebnf",   iso_ebnf_input, iso_ebnf_output,    iso_ebnf_ast_unsupported, 0 },
	{ "rbnf",       rbnf_input,     rbnf_output,        rbnf_ast_unsupported, 0 },
	{ "sid",        NULL,           sid_output,         sid_ast_unsupported, 0 },
	{ "dot",        NULL,           dot_output,         0, 0 },
	{ "rrdot",      NULL,           rrdot_output,       0, 0 },
	{ "rrdump",     NULL,           rrdump_output,      0, 0 },
	{ "rrtdump",    NULL,           rrtdump_output,     0, 0 },
	{ "rrparcon",   NULL,           rrparcon_output,    rrparcon_ast_unsupported, rrparcon_rrd_unsupported },
	{ "rrll",     NULL,           rrll_output,     rrll_ast_unsupported, rrll_rrd_unsupported     },
	{ "rrta",     NULL,           rrta_output,     rrta_ast_unsupported, rrta_rrd_unsupported     },
	{ "rrtext",   NULL,           rrtext_output,   0, 0 },
	{ "rrutf8",   NULL,           rrutf8_output,   0, 0 },
	{ "svg",      NULL,           svg_output,      0, 0 },
	{ "html5",    NULL,           html5_output,    0, 0 },
	{ "xhtml5",   NULL,           xhtml5_output,   0, 0 },
    { "json",       NULL,           json_output,        json_ast_unsupported, 0 }
};

enum io_dir {
	IO_IN,
	IO_OUT
};

static void
xusage(void)
{
	printf("usage: kgt [-nu] [-w <whitelist>] [-l <language>] [ -e <language> ]\n");
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
	const char *filter;
	parsing_error_queue errors = NULL;

	in  = lang(IO_IN, "bnf");
	out = in;
	filter = NULL;

	{
		int c;

		while (c = getopt(argc, argv, "hw:c:gnl:e:u"), c != -1) {
			switch (c) {
			case 'l': in  = lang(IO_IN,  optarg); break;
			case 'e': out = lang(IO_OUT, optarg); break;

			case 'c': css_file = optarg; break;
			case 'w': filter   = optarg; break; /* comma-separated whitelist of rule names */

			case 'g': debug           = 1; break;
			case 'n': prettify        = 0; break;
			case 'u': allow_undefined = 0; break;

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

	g = in->in(kgt_fgetc, stdin, &errors);

	{
		int error_count = 0;

		while (errors) {
			parsing_error error;
			parsing_error_queue_pop(&errors, &error);

			fprintf(stderr, "%u:%u: %s\n", error.line, error.col, error.description);
			error_count += 1;
		}

		if (error_count != 0) {
			fprintf(stderr, "KGT: Exiting. %d errors reported\n", error_count);
			exit(EXIT_FAILURE);
		}
	}

	{
		unsigned v;

		for (v = out->ast_unsupported; v != 0; v &= v - 1) {
			/* TODO: expose these rewritings as CLI options too; set as bits in v */
			/* TODO: option to query if output is possible without rewriting */
			switch (v & -v) {
			case FEATURE_AST_CI_LITERAL:
			  if (!rewrite_ci_literals(g)) {
				return EXIT_FAILURE;
			  }
			  break;
			case FEATURE_AST_INVISIBLE:  rewrite_invisible(g);   break;

			case FEATURE_AST_BINARY:
				if (ast_binary(g)) {
					fprintf(stderr, "Binary strings not supported for this output language\n");
				}
				break;
			}
		}
	}

	if (filter != NULL) {
		struct ast_rule *new, **tail;
		struct ast_rule *p, *next;

		new  = NULL;
		tail = &new;

		for (p = g; p != NULL; p = next) {
			char *tmp, *save;
			const char *t;

			next = p->next;

			tmp = xstrdup(filter);

			for (t = strtok_r(tmp, ",", &save); t != NULL; t = strtok_r(NULL, ",", &save)) {
				if (0 == strcmp(p->name, t)) {
					p->next = *tail;
					*tail = p;
					break;
				}

				/* TODO: otherwise free *p */
			}

			free(tmp);
		}

		g = new;
	}

	if (!out->out(g)) {
		return EXIT_FAILURE;
	}

	/* TODO: free ast */

	return EXIT_SUCCESS;
}
