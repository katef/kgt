/*
 * Automatically generated from the files:
 *	src/ebnf/parser.sid
 * and
 *	src/parser.act
 * by:
 *	sid
 */

/* BEGINNING OF HEADER */

#line 87 "src/parser.act"


	#include <assert.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <stdio.h>

	#include "../ast.h"
	#include "../xalloc.h"

	#ifndef FORM
	#error FORM required
	#endif

	#define PASTE(a, b) a ## b
	#define CAT(a, b)   PASTE(a, b)

	#define LX_PREFIX CAT(lx_, FORM)

	#define LX_TOKEN  CAT(LX_PREFIX, _token)
	#define LX_STATE  CAT(LX_PREFIX, _lx)
	#define LX_NEXT   CAT(LX_PREFIX, _next)
	#define LX_INIT   CAT(LX_PREFIX, _init)

	#define FORM_INPUT CAT(FORM, _input)

	/* XXX: get rid of this; use same %entry% for all grammars */
	#define FORM_ENTRY CAT(prod_, FORM)

	#include "parser.h"
	#include "lexer.h"

	#include "io.h"

	typedef char         map_char;
	typedef const char * map_string;
	typedef unsigned int map_number;

	typedef struct ast_term * map_term;
	typedef struct ast_alt * map_alt;

	struct act_state {
		enum LX_TOKEN lex_tok;
		enum LX_TOKEN lex_tok_save;
	};

	struct lex_state {
		struct LX_STATE lx;
		struct lx_dynbuf buf;

		int (*f)(void *opaque);
		void *opaque;

		/* TODO: use lx's generated conveniences for the pattern buffer */
		char a[512];
		char *p;
	};

	#define CURRENT_TERMINAL (act_state->lex_tok)
	#define ERROR_TERMINAL   (TOK_ERROR)
	#define ADVANCE_LEXER    do { act_state->lex_tok = LX_NEXT(&lex_state->lx); } while (0)
	#define SAVE_LEXER(tok)  do { act_state->lex_tok_save = act_state->lex_tok; \
	                              act_state->lex_tok = tok;                     } while (0)
	#define RESTORE_LEXER    do { act_state->lex_tok = act_state->lex_tok_save; } while (0)

	static void
	err(const struct lex_state *lex_state, const char *fmt, ...)
	{
		va_list ap;

		assert(lex_state != NULL);

		va_start(ap, fmt);
		fprintf(stderr, "%u:%u: ",
			lex_state->lx.start.line, lex_state->lx.start.col);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		va_end(ap);
	}

	static void
	err_expected(const struct lex_state *lex_state, const char *token)
	{
		err(lex_state, "Syntax error: expected %s", token);
		exit(EXIT_FAILURE);
	}

#line 102 "src/ebnf/parser.c"


#ifndef ERROR_TERMINAL
#error "-s no-numeric-terminals given and ERROR_TERMINAL is not defined"
#endif

/* BEGINNING OF FUNCTION DECLARATIONS */

static void prod_factor(lex_state, act_state, map_term *);
extern void prod_ebnf(lex_state, act_state, map_rule *);
static void prod_list_Hof_Hterms(lex_state, act_state, map_term *);
static void prod_list_Hof_Hrules(lex_state, act_state, map_rule *);
static void prod_list_Hof_Halts(lex_state, act_state, map_alt *);
static void prod_body(lex_state, act_state);
static void prod_term(lex_state, act_state, map_term *);
static void prod_rule(lex_state, act_state, map_rule *);
static void prod_74(lex_state, act_state, map_rule *);
static void prod_75(lex_state, act_state, map_term *, map_alt *);
static void prod_76(lex_state, act_state, map_term *);
static void prod_repeatable_Hfactor(lex_state, act_state, map_term *);

/* BEGINNING OF STATIC VARIABLES */


/* BEGINNING OF FUNCTION DEFINITIONS */

static void
prod_factor(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_STARTGROUP):
		{
			map_alt ZIa;
			map_number ZImin;
			map_number ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDGROUP):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 253 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->next = NULL;
		(ZIt)->u.group = (ZIa);
	
#line 162 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: rep-one */
			{
#line 186 "src/parser.act"

		(ZImin) = 1;
		(ZImax) = 1;
	
#line 172 "src/ebnf/parser.c"
			}
			/* END OF ACTION: rep-one */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 206 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 184 "src/ebnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_STARTOPT):
		{
			map_alt ZIa;
			map_number ZImin;
			map_number ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDOPT):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 253 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->next = NULL;
		(ZIt)->u.group = (ZIa);
	
#line 216 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: rep-zero-or-one */
			{
#line 201 "src/parser.act"

		(ZImin) = 0;
		(ZImax) = 1;
	
#line 226 "src/ebnf/parser.c"
			}
			/* END OF ACTION: rep-zero-or-one */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 206 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 238 "src/ebnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_STARTSTAR):
		{
			map_alt ZIa;
			map_number ZImin;
			map_number ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDSTAR):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 253 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->next = NULL;
		(ZIt)->u.group = (ZIa);
	
#line 270 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: rep-zero-or-more */
			{
#line 196 "src/parser.act"

		(ZImin) = 0;
		(ZImax) = 0;
	
#line 280 "src/ebnf/parser.c"
			}
			/* END OF ACTION: rep-zero-or-more */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 206 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 292 "src/ebnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	default:
		{
			map_number ZImin;
			map_number ZImax;

			prod_term (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: rep-one */
			{
#line 186 "src/parser.act"

		(ZImin) = 1;
		(ZImax) = 1;
	
#line 314 "src/ebnf/parser.c"
			}
			/* END OF ACTION: rep-one */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 206 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 326 "src/ebnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

void
prod_ebnf(lex_state lex_state, act_state act_state, map_rule *ZOl)
{
	map_rule ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_list_Hof_Hrules (lex_state, act_state, &ZIl);
		switch (CURRENT_TERMINAL) {
		case (TOK_EOF):
			break;
		case (ERROR_TERMINAL):
			RESTORE_LEXER;
			goto ZL1;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
	}
	goto ZL0;
ZL1:;
	{
		/* BEGINNING OF ACTION: make-empty-rule */
		{
#line 274 "src/parser.act"

		(ZIl) = NULL;
	
#line 372 "src/ebnf/parser.c"
		}
		/* END OF ACTION: make-empty-rule */
		/* BEGINNING OF ACTION: err-syntax */
		{
#line 297 "src/parser.act"

		err(lex_state, "Syntax error");
		exit(EXIT_FAILURE);
	
#line 382 "src/ebnf/parser.c"
		}
		/* END OF ACTION: err-syntax */
	}
ZL0:;
	*ZOl = ZIl;
}

static void
prod_list_Hof_Hterms(lex_state lex_state, act_state act_state, map_term *ZOl)
{
	map_term ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_repeatable_Hfactor (lex_state, act_state, &ZIl);
		prod_76 (lex_state, act_state, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_list_Hof_Hrules(lex_state lex_state, act_state act_state, map_rule *ZOl)
{
	map_rule ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_rule (lex_state, act_state, &ZIl);
		prod_74 (lex_state, act_state, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_list_Hof_Halts(lex_state lex_state, act_state act_state, map_alt *ZOl)
{
	map_alt ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_term ZIt;

		prod_list_Hof_Hterms (lex_state, act_state, &ZIt);
		prod_75 (lex_state, act_state, &ZIt, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_body(lex_state lex_state, act_state act_state)
{
ZL2_body:;
	switch (CURRENT_TERMINAL) {
	case (TOK_CHAR):
		{
			map_char ZIc;

			/* BEGINNING OF INLINE: 55 */
			{
				{
					switch (CURRENT_TERMINAL) {
					case (TOK_CHAR):
						/* BEGINNING OF EXTRACT: CHAR */
						{
#line 141 "src/parser.act"

		assert(strlen(lex_state->buf.a) == 1);

		ZIc = lex_state->buf.a[0];
	
#line 486 "src/ebnf/parser.c"
						}
						/* END OF EXTRACT: CHAR */
						break;
					default:
						goto ZL1;
					}
					ADVANCE_LEXER;
				}
			}
			/* END OF INLINE: 55 */
			/* BEGINNING OF ACTION: pattern-char */
			{
#line 163 "src/parser.act"

		/* TODO */
		*lex_state->p++ = (ZIc);
	
#line 504 "src/ebnf/parser.c"
			}
			/* END OF ACTION: pattern-char */
			/* BEGINNING OF INLINE: body */
			goto ZL2_body;
			/* END OF INLINE: body */
		}
		/*UNREACHED*/
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

static void
prod_term(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_IDENT):
		{
			map_string ZIs;

			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 145 "src/parser.act"

		ZIs = xstrdup(lex_state->buf.a);
		if (ZIs == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 543 "src/ebnf/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-rule-term */
			{
#line 239 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_RULE;
		(ZIt)->next = NULL;
		(ZIt)->u.name = (ZIs);
	
#line 556 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-rule-term */
		}
		break;
	case (TOK_LITERAL): case (TOK_CHAR):
		{
			map_string ZIs;

			prod_body (lex_state, act_state);
			switch (CURRENT_TERMINAL) {
			case (TOK_LITERAL):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: pattern-buffer */
			{
#line 175 "src/parser.act"

		/* TODO */
		*lex_state->p++ = '\0';

		/*
		 * Note we strdup() here because the grammar permits adjacent patterns,
		 * and so the pattern buffer will be overwritten by the LL(1) one-token
		 * lookahead.
		 */
		(ZIs) = xstrdup(lex_state->a);
		if ((ZIs) == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		lex_state->p = lex_state->a;
	
#line 596 "src/ebnf/parser.c"
			}
			/* END OF ACTION: pattern-buffer */
			/* BEGINNING OF ACTION: make-literal-term */
			{
#line 246 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_TERMINAL;	/* TODO: rename to literal or vice-versa, perhaps */
		(ZIt)->next = NULL;
		(ZIt)->u.literal = (ZIs);
	
#line 608 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-literal-term */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-empty-term */
			{
#line 233 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_EMPTY;
		(ZIt)->next = NULL;
	
#line 623 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-empty-term */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

static void
prod_rule(lex_state lex_state, act_state act_state, map_rule *ZOr)
{
	map_rule ZIr;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_string ZIs;
		map_alt ZIa;

		switch (CURRENT_TERMINAL) {
		case (TOK_IDENT):
			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 145 "src/parser.act"

		ZIs = xstrdup(lex_state->buf.a);
		if (ZIs == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 663 "src/ebnf/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			break;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
		/* BEGINNING OF INLINE: 70 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_EQUALS):
					break;
				default:
					goto ZL3;
				}
				ADVANCE_LEXER;
			}
			goto ZL2;
		ZL3:;
			{
				/* BEGINNING OF ACTION: err-expected-equals */
				{
#line 309 "src/parser.act"

		err_expected(lex_state, "production rule assignment");
	
#line 691 "src/ebnf/parser.c"
				}
				/* END OF ACTION: err-expected-equals */
			}
		ZL2:;
		}
		/* END OF INLINE: 70 */
		prod_list_Hof_Halts (lex_state, act_state, &ZIa);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
		/* BEGINNING OF ACTION: make-rule */
		{
#line 267 "src/parser.act"

		(ZIr) = xmalloc(sizeof *(ZIr));
		(ZIr)->name = (ZIs);
		(ZIr)->alts = (ZIa);
		(ZIr)->next = NULL;
	
#line 712 "src/ebnf/parser.c"
		}
		/* END OF ACTION: make-rule */
		/* BEGINNING OF INLINE: 71 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_SEP):
					break;
				default:
					goto ZL5;
				}
				ADVANCE_LEXER;
			}
			goto ZL4;
		ZL5:;
			{
				/* BEGINNING OF ACTION: err-expected-sep */
				{
#line 305 "src/parser.act"

		err_expected(lex_state, "production rule separator");
	
#line 735 "src/ebnf/parser.c"
				}
				/* END OF ACTION: err-expected-sep */
			}
		ZL4:;
		}
		/* END OF INLINE: 71 */
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOr = ZIr;
}

static void
prod_74(lex_state lex_state, act_state act_state, map_rule *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_IDENT):
		{
			map_rule ZIr;

			prod_list_Hof_Hrules (lex_state, act_state, &ZIr);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-rule-to-list */
			{
#line 289 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIr);
	
#line 771 "src/ebnf/parser.c"
			}
			/* END OF ACTION: add-rule-to-list */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

static void
prod_75(lex_state lex_state, act_state act_state, map_term *ZIt, map_alt *ZOl)
{
	map_alt ZIl;

	switch (CURRENT_TERMINAL) {
	case (TOK_ALT):
		{
			map_alt ZIa;

			/* BEGINNING OF INLINE: 67 */
			{
				{
					switch (CURRENT_TERMINAL) {
					case (TOK_ALT):
						break;
					default:
						goto ZL3;
					}
					ADVANCE_LEXER;
				}
				goto ZL2;
			ZL3:;
				{
					/* BEGINNING OF ACTION: err-expected-alt */
					{
#line 301 "src/parser.act"

		err_expected(lex_state, "alternative separator");
	
#line 817 "src/ebnf/parser.c"
					}
					/* END OF ACTION: err-expected-alt */
				}
			ZL2:;
			}
			/* END OF INLINE: 67 */
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: make-alt */
			{
#line 261 "src/parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 837 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-alt */
			/* BEGINNING OF ACTION: add-alt-to-list */
			{
#line 284 "src/parser.act"

		assert((ZIl)->next == NULL);
		(ZIl)->next = (ZIa);
	
#line 847 "src/ebnf/parser.c"
			}
			/* END OF ACTION: add-alt-to-list */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-alt */
			{
#line 261 "src/parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 862 "src/ebnf/parser.c"
			}
			/* END OF ACTION: make-alt */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_76(lex_state lex_state, act_state act_state, map_term *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_CAT):
		{
			map_term ZIt;

			ADVANCE_LEXER;
			prod_list_Hof_Hterms (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-term-to-list */
			{
#line 279 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIt);
	
#line 899 "src/ebnf/parser.c"
			}
			/* END OF ACTION: add-term-to-list */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

static void
prod_repeatable_Hfactor(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_NUMBER):
		{
			map_number ZIn;

			/* BEGINNING OF EXTRACT: NUMBER */
			{
#line 153 "src/parser.act"

		ZIn = strtoul(lex_state->buf.a, NULL, 10);
		/* TODO: range check */
	
#line 932 "src/ebnf/parser.c"
			}
			/* END OF EXTRACT: NUMBER */
			ADVANCE_LEXER;
			switch (CURRENT_TERMINAL) {
			case (TOK_STAR):
				break;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			prod_factor (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: mul-repeat */
			{
#line 213 "src/parser.act"

		assert((ZIn) > 0);

		/*
		 * This multiplies a term's {min,max} pair as if the term were
		 * repeated multiple times inline. Possible situations here are:
		 *
		 *   {0,0} * n => {0,0}
		 *   {0,1} * n => {0,n}
		 *   {1,0} * n => {n,0}
		 *   {i,j} * n => {in,jn} where j >= i
		 *
		 * Note a max of 0 indicates no limit on the number of repetitions.
		 */

		(ZIt)->min *= (ZIn);
		(ZIt)->max *= (ZIn);
	
#line 969 "src/ebnf/parser.c"
			}
			/* END OF ACTION: mul-repeat */
		}
		break;
	default:
		{
			prod_factor (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

/* BEGINNING OF TRAILER */

#line 380 "src/parser.act"


	static int
	lgetc(struct LX_STATE *lx)
	{
		const struct lex_state *lex_state;

		assert(lx != NULL);
		assert(lx->opaque != NULL);

		lex_state = lx->opaque;

		assert(lex_state->f != NULL);

		return lex_state->f(lex_state->opaque);
	}

	struct ast_rule *
	FORM_INPUT(int (*f)(void *opaque), void *opaque)
	{
		struct act_state  act_state_s;
		struct act_state *act_state;
		struct lex_state  lex_state_s;
		struct lex_state *lex_state;

		struct LX_STATE *lx;
		struct ast_rule *new;

		assert(f != NULL);

		new = NULL;

		lex_state    = &lex_state_s;
		lex_state->p = lex_state->a;

		lx = &lex_state->lx;

		LX_INIT(lx);

		lx->lgetc  = lgetc;
		lx->opaque = lex_state;

		lex_state->f       = f;
		lex_state->opaque  = opaque;

		lex_state->buf.a   = NULL;
		lex_state->buf.len = 0;

		/* XXX: unneccessary since we're lexing from a string */
		lx->buf   = &lex_state->buf;
		lx->push  = CAT(LX_PREFIX, _dynpush);
		lx->pop   = CAT(LX_PREFIX, _dynpop);
		lx->clear = CAT(LX_PREFIX, _dynclear);
		lx->free  = CAT(LX_PREFIX, _dynfree);

	/* XXX */
	lx->free = NULL;

		/* This is a workaround for ADVANCE_LEXER assuming a pointer */
		act_state = &act_state_s;

		ADVANCE_LEXER;
		FORM_ENTRY(lex_state, act_state, &new);

		/* TODO: handle error */

		return new;
	}

#line 1066 "src/ebnf/parser.c"

/* END OF FILE */
