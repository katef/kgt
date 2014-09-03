/* $Id$ */

#define _POSIX_C_SOURCE 2

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* See %maps% - This is given for the entry point into the grammar */
/* TODO: how to do this nicely? permit more complex types for SID's .act files? */
typedef struct ast_production * map_production;

#include "bnf/lexer.h"
#include "bnf/parser.h"
#include "bnf/output.h"

#include "wsn/lexer.h"
#include "wsn/parser.h"
#include "wsn/output.h"

#include "ebnf/lexer.h"
#include "ebnf/parser.h"
#include "ebnf/output.h"

#include "sid/output.h"

#include "trd/output.h"

#include "dot/output.h"

#include "rrd/output.h"

#include "io.h"
#include "ast.h"
#include "tokens.h"
#include "xalloc.h"

int act_currenttoken;

enum lang {
	LANG_BNF,
	LANG_WSN,
	LANG_EBNF,
	LANG_SID,
	LANG_TRD,
	LANG_DOT,
	LANG_RRD
} input, output;

union {
	struct bnf_state  bnf_state;
	struct wsn_state  wsn_state;
	struct ebnf_state ebnf_state;
} state;

int
act_next(void)
{
	int t;

	switch (input) {
	case LANG_BNF:  t = bnf_next(&state.bnf_state);   break;
	case LANG_WSN:  t = wsn_next(&state.wsn_state);   break;
	case LANG_EBNF: t = ebnf_next(&state.ebnf_state); break;
	}

	if (t == tok_unrecognised) {
		xerror("unrecognised token on line %d", io_line);
	}

	return t;
}

static void
xusage(void)
{
	printf("usage: kgt [-n] [-l <language>] [ -e <language> ]\n");
	exit(EXIT_FAILURE);
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

	io_fin = stdin;
	io_line = 1;

	/* TODO: default format from extension. override with -l */
	/* TODO: array of structs containing lexer stuff? probably not */

	grammar = NULL;
	switch (input) {
	case LANG_BNF:
		bnf_init(&state.bnf_state);
		act_currenttoken = act_next();
		prod_bnf_Hgrammar(&grammar);
		break;

	case LANG_WSN:
		wsn_init(&state.wsn_state);
		act_currenttoken = act_next();
		prod_wsn_Hgrammar(&grammar);
		break;

	case LANG_EBNF:
		ebnf_init(&state.ebnf_state);
		act_currenttoken = act_next();
		prod_ebnf_Hgrammar(&grammar);
		break;

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

