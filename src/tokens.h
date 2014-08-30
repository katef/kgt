/* $Id$ */

/*
 * Token values. These are centralised for all parsers.
 * Each value represents one lexeme.
 */

#ifndef KGT_TOKENS_H
#define KGT_TOKENS_H

enum tok {
	tok_error,	/* for SID */
	tok_unrecognised,

	tok_equals,
	tok_alt,
	tok_sep,
	tok_except,

	tok_star,
	tok_cat,
	tok_opt,

	tok_start_Hgroup,
	tok_end_Hgroup,
	tok_start_Hopt,
	tok_end_Hopt,
	tok_start_Hstar,
	tok_end_Hstar,

	tok_empty,
	tok_name,
	tok_literal,
	tok_number
};

#endif

