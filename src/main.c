/* $Id$ */

/*
 * Kate's Grammar Tool.
 */

#define _POSIX_C_SOURCE 2

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

/* See %maps% - This is given for the entry point into the grammar */
/* TODO: how to do this nicely? permit more complex types for SID's .act files? */
typedef struct ast_production * map_production;

#include "bnf/bnf_lexer.h"
#include "bnf/bnf_parser.h"
#include "bnf/bnf_output.h"

#include "wsn/wsn_lexer.h"
#include "wsn/wsn_parser.h"
#include "wsn/wsn_output.h"

#include "ebnf/ebnf_lexer.h"
#include "ebnf/ebnf_parser.h"
#include "ebnf/ebnf_output.h"

#include "sid/sid_output.h"

#include "trd/trd_output.h"

#include "io.h"
#include "ast.h"
#include "main.h"
#include "tokens.h"

int act_currenttoken;

enum language {
	LANG_BNF,
	LANG_WSN,
	LANG_EBNF,
	LANG_SID,
	LANG_TRD
} input, output;

union {
	struct bnf_state bnf_state;
	struct wsn_state wsn_state;
	struct ebnf_state ebnf_state;
} state;

void *xmalloc(size_t size) {
	void *new;

	new = malloc(size);
	if (new == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	return new;
}

char *xstrdup(const char *s) {
	char *new;

	new = xmalloc(strlen(s) + 1);
	return strcpy(new, s);
}

void xerror(const char *msg, ...) {
	va_list ap;

	fprintf(stderr, "kgt: ");

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(EXIT_FAILURE);
}

int act_next(void) {
	int t;

	switch (input) {
	case LANG_BNF:
		t = bnf_next(&state.bnf_state);
		break;

	case LANG_WSN:
		t = wsn_next(&state.wsn_state);
		break;

	case LANG_EBNF:
		t = ebnf_next(&state.ebnf_state);
		break;
	}

	if (t == tok_unrecognised) {
		xerror("unrecognised token on line %d", io_line);
	}

	return t;
}

static void xusage(void) {
	printf("usage: kgt [-l <language>] [ -e <language> ]\n");
	exit(EXIT_FAILURE);
}

static enum language languageopt(const char *optarg) {
	if (0 == strcasecmp(optarg, "BNF")) {
		return LANG_BNF;
	} else if (0 == strcasecmp(optarg, "WSN")) {
		return LANG_WSN;
	} else if (0 == strcasecmp(optarg, "EBNF")) {
		return LANG_EBNF;
	} else if (0 == strcasecmp(optarg, "SID")) {
		return LANG_SID;
	} else if (0 == strcasecmp(optarg, "TRD")) {
		return LANG_TRD;
	}

	xerror("unrecognised language");
}

int main(int argc, char *argv[]) {
	struct ast_production *grammar;

	input = LANG_BNF;
	output = input;

	{
		int c;

		while ((c = getopt(argc, argv, "hl:e:")) != -1) {
			switch (c) {
			case 'l':
				input = languageopt(optarg);
				break;

			case 'e':
				output = languageopt(optarg);
				break;

			case '?':
			default:
				xusage();
			}
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		xusage();
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
	case LANG_BNF:
		bnf_output(grammar);
		break;

	case LANG_WSN:
		wsn_output(grammar);
		break;

	case LANG_EBNF:
		ebnf_output(grammar);
		break;

	case LANG_SID:
		sid_output(grammar);
		break;

	case LANG_TRD:
		trd_output(grammar);
		break;

	default:
		fprintf(stderr, "Unsupported output language\n");
	}

	return EXIT_SUCCESS;
}

