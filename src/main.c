/* $Id$ */

#define _POSIX_C_SOURCE 2

#include <unistd.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bnf/input.h"
#include "wsn/input.h"
#include "ebnf/input.h"

#include "bnf/output.h"
#include "wsn/output.h"
#include "ebnf/output.h"
#include "sid/output.h"
#include "trd/output.h"
#include "dot/output.h"
#include "rrd/output.h"

#include "ast.h"
#include "xalloc.h"

struct lang {
	const char *name;
	struct ast_production *(*in)(int (*f)(void *), void *);
	void (*out)(struct ast_production *);
} lang[] = {
	{ "bnf",  bnf_input,  bnf_output  },
	{ "wsn",  wsn_input,  wsn_output  },
	{ "ebnf", ebnf_input, ebnf_output },
	{ "sid",  NULL,       sid_output  },
	{ "trd",  NULL,       trd_output  },
	{ "dot",  NULL,       dot_output  },
	{ "rrd",  NULL,       rrd_output  }
};

struct lang *in, *out;

static void
xusage(void)
{
	printf("usage: kgt [-n] [-l <language>] [ -e <language> ]\n");
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

static struct lang *
findlang(const char *s)
{
	size_t i;

	for (i = 0; i < sizeof lang / sizeof *lang; i++) {
		if (0 == strcmp(s, lang[i].name)) {
			return &lang[i];
		}
	}

	fprintf(stderr, "Unsupported language: %s\n", s);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct ast_production *g;

	in  = findlang("bnf");
	out = in;

	{
		int c;

		while ((c = getopt(argc, argv, "hnl:e:")) != -1) {
			switch (c) {
			case 'l': in  = findlang(optarg); break;
			case 'e': out = findlang(optarg); break;

			case 'n':
				beautify = 0;
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

	return EXIT_SUCCESS;
}

