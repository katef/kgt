/* $Id$ */

#define _POSIX_C_SOURCE 2

#include <unistd.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bnf/io.h"
#include "wsn/io.h"
#include "ebnf/io.h"
#include "sid/io.h"
#include "trd/io.h"
#include "dot/io.h"
#include "rrdot/io.h"
#include "rrdump/io.h"
#include "rrtext/io.h"

#include "ast.h"
#include "xalloc.h"

int prettify = 1;
int allow_undefined = 1;

struct io {
	const char *name;
	struct ast_rule *(*in)(int (*f)(void *), void *);
	void (*out)(const struct ast_rule *);
} io[] = {
	{ "bnf",    bnf_input,  bnf_output    },
	{ "wsn",    wsn_input,  wsn_output    },
	{ "ebnf",   ebnf_input, ebnf_output   },
	{ "sid",    NULL,       sid_output    },
	{ "trd",    NULL,       trd_output    },
	{ "dot",    NULL,       dot_output    },
	{ "rrdot",  NULL,       rrdot_output  },
	{ "rrdump", NULL,       rrdump_output },
	{ "rrtext", NULL,       rrtext_output }
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
lang(const char *s)
{
	size_t i;

	for (i = 0; i < sizeof io / sizeof *io; i++) {
		if (0 == strcmp(s, io[i].name)) {
			return &io[i];
		}
	}

	fprintf(stderr, "Unsupported language: %s\n", s);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct ast_rule *g;
	struct io *in, *out;

	in  = lang("bnf");
	out = in;

	{
		int c;

		while ((c = getopt(argc, argv, "hnl:e:u")) != -1) {
			switch (c) {
			case 'l': in  = lang(optarg); break;
			case 'e': out = lang(optarg); break;

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

	g = in->in(kgt_fgetc, stdin);

	out->out(g);

	/* TODO: free ast */

	return EXIT_SUCCESS;
}

