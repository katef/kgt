/* $Id$ */

#define _POSIX_C_SOURCE 2

#include <unistd.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* See %maps% - This is given for the entry point into the grammar */
/* TODO: how to do this nicely? permit more complex types for SID's .act files? */
typedef struct ast_production * map_production;

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

enum lang {
	LANG_BNF,
	LANG_WSN,
	LANG_EBNF,
	LANG_SID,
	LANG_TRD,
	LANG_DOT,
	LANG_RRD
} input, output;

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

static enum lang
lang(const char *optarg)
{
	size_t i;

	struct {
		enum lang lang;
		const char *name;
	} a[] = {
		{ LANG_BNF,  "bnf"  },
		{ LANG_WSN,  "wsn"  },
		{ LANG_EBNF, "ebnf" },
		{ LANG_SID,  "sid"  },
		{ LANG_TRD,  "trd"  },
		{ LANG_DOT,  "dot"  },
		{ LANG_RRD,  "rrd"  },
	};

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (0 == strcmp(optarg, a[i].name)) {
			return a[i].lang;
		}
	}

	xerror("unrecognised language");
}

int
main(int argc, char *argv[])
{
	struct ast_production *grammar;
	int beautify = 1;

	input = LANG_BNF;
	output = input;

	{
		int c;

		while ((c = getopt(argc, argv, "hnl:e:")) != -1) {
			switch (c) {
			case 'l':
				input = lang(optarg);
				break;

			case 'e':
				output = lang(optarg);
				break;

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

	switch (input) {
	case LANG_BNF:  grammar = bnf_input (kgt_fgetc, stdin); break;
	case LANG_WSN:  grammar = wsn_input (kgt_fgetc, stdin); break;
	case LANG_EBNF: grammar = ebnf_input(kgt_fgetc, stdin); break;

	default:
		fprintf(stderr, "Unsupported input language\n");
	}

	switch (output) {
	case LANG_BNF:  bnf_output (grammar); break;
	case LANG_WSN:  wsn_output (grammar); break;
	case LANG_EBNF: ebnf_output(grammar); break;
	case LANG_SID:  sid_output (grammar); break;
	case LANG_TRD:  trd_output (grammar); break;
	case LANG_DOT:  dot_output (grammar); break;
	case LANG_RRD:  rrd_output (grammar, beautify); break;

	default:
		fprintf(stderr, "Unsupported output language\n");
	}

	return EXIT_SUCCESS;
}

