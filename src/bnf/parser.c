/*
 * Automatically generated from the files:
 *	bnf_parser.sid
 * and
 *	../parser.act
 * by:
 *	sid
 */

/* BEGINNING OF HEADER */

#line 40 "../parser.act"


	#include <stdlib.h>
	#include <string.h>
	#include <assert.h>
	#include <ctype.h>

	#include "../io.h"
	#include "../ast.h"
	#include "../tokens.h"
	#include "../xalloc.h"

	/* See main.c */
	extern int act_currenttoken;
	extern int act_next(void);

	int act_savedtoken;

	/* Interfaces required by SID's generated parser */
	#define ERROR_TERMINAL   (tok_error)
	#define CURRENT_TERMINAL act_currenttoken
	#define ADVANCE_LEXER    do { act_currenttoken = act_next(); } while (0)
	#define SAVE_LEXER(t)    do { act_savedtoken = act_currenttoken; act_currenttoken = (t); } while (0)
	#define RESTORE_LEXER    do { act_currenttoken = act_savedtoken; } while (0)

	/* See %maps% */
	typedef const char * map_string;
	typedef unsigned int map_number;

	typedef struct ast_term * map_term;
	typedef struct ast_alt * map_alt;
	typedef struct ast_production * map_production;

	static void
	expected(const char *msg)
	{
		if (msg == NULL) {
			xerror("syntax error on line %u", io_line);
		} else {
			xerror("syntax error on line %u: expected %s", io_line, msg);
		}
	}

	static void
	rtrim(char *s)
	{
		char *p = s + strlen(s) - 1;

		assert(strlen(s) > 0);

		while (p >= s && isspace((unsigned char) *p)) {
			*p-- = '\0';
		}
	}

#line 69 "bnf_parser.c"


#ifndef ERROR_TERMINAL
#error "-s no-numeric-terminals given and ERROR_TERMINAL is not defined"
#endif

/* BEGINNING OF FUNCTION DECLARATIONS */

static void prod_list_Hof_Hterms(map_term *);
static void prod_list_Hof_Halts(map_alt *);
static void prod_48(map_production *);
static void prod_49(map_term *, map_alt *);
static void prod_50(map_term *);
static void prod_list_Hof_Hproductions(map_production *);
static void prod_term(map_term *);
static void prod_production(map_production *);
extern void prod_bnf_Hgrammar(map_production *);

/* BEGINNING OF STATIC VARIABLES */


/* BEGINNING OF FUNCTION DEFINITIONS */

static void
prod_list_Hof_Hterms(map_term *ZOl)
{
	map_term ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_term (&ZIl);
		prod_50 (&ZIl);
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
prod_list_Hof_Halts(map_alt *ZOl)
{
	map_alt ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_term ZIt;

		prod_list_Hof_Hterms (&ZIt);
		prod_49 (&ZIt, &ZIl);
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
prod_48(map_production *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (tok_name):
		{
			map_production ZIp;

			prod_list_Hof_Hproductions (&ZIp);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-production-to-list */
			{
#line 203 "../parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIp);
	
#line 163 "bnf_parser.c"
			}
			/* END OF ACTION: add-production-to-list */
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
prod_49(map_term *ZIt, map_alt *ZOl)
{
	map_alt ZIl;

	switch (CURRENT_TERMINAL) {
	case (tok_alt):
		{
			map_alt ZIa;

			/* BEGINNING OF INLINE: 39 */
			{
				{
					switch (CURRENT_TERMINAL) {
					case (tok_alt):
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
#line 210 "../parser.act"

		expected("alternative separator");
	
#line 209 "bnf_parser.c"
					}
					/* END OF ACTION: err-expected-alt */
				}
			ZL2:;
			}
			/* END OF INLINE: 39 */
			prod_list_Hof_Halts (&ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: make-alt */
			{
#line 144 "../parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 229 "bnf_parser.c"
			}
			/* END OF ACTION: make-alt */
			/* BEGINNING OF ACTION: add-alt-to-list */
			{
#line 198 "../parser.act"

		assert((ZIl)->next == NULL);
		(ZIl)->next = (ZIa);
	
#line 239 "bnf_parser.c"
			}
			/* END OF ACTION: add-alt-to-list */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-alt */
			{
#line 144 "../parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 254 "bnf_parser.c"
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
prod_50(map_term *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (tok_empty): case (tok_name): case (tok_literal):
		{
			map_term ZIt;

			prod_list_Hof_Hterms (&ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-term-to-list */
			{
#line 193 "../parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIt);
	
#line 290 "bnf_parser.c"
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
prod_list_Hof_Hproductions(map_production *ZOl)
{
	map_production ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_production (&ZIl);
		prod_48 (&ZIl);
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
prod_term(map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (tok_empty):
		{
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-empty-term */
			{
#line 121 "../parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_EMPTY;
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 348 "bnf_parser.c"
			}
			/* END OF ACTION: make-empty-term */
		}
		break;
	case (tok_literal):
		{
			map_string ZIl;

			/* BEGINNING OF EXTRACT: literal */
			{
#line 100 "../parser.act"

		assert(strlen(io_buffer) > 0);
		ZIl = xstrdup(io_buffer);
		io_flush();
	
#line 365 "bnf_parser.c"
			}
			/* END OF EXTRACT: literal */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-literal-term */
			{
#line 128 "../parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_TERMINAL;	/* TODO rename to literal or vice-versa, perhaps */
		(ZIt)->u.literal = (ZIl);
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 379 "bnf_parser.c"
			}
			/* END OF ACTION: make-literal-term */
		}
		break;
	case (tok_name):
		{
			map_string ZIn;

			/* BEGINNING OF EXTRACT: name */
			{
#line 94 "../parser.act"

		assert(strlen(io_buffer) > 0);
		assert(!isspace((unsigned char) io_buffer[0]));
		rtrim(io_buffer);
		ZIn = xstrdup(io_buffer);
		io_flush();
	
#line 398 "bnf_parser.c"
			}
			/* END OF EXTRACT: name */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-production-term */
			{
#line 136 "../parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_PRODUCTION;
		(ZIt)->u.name = (ZIn);
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 412 "bnf_parser.c"
			}
			/* END OF ACTION: make-production-term */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		goto ZL1;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

static void
prod_production(map_production *ZOp)
{
	map_production ZIp;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_string ZIn;
		map_alt ZIa;

		switch (CURRENT_TERMINAL) {
		case (tok_name):
			/* BEGINNING OF EXTRACT: name */
			{
#line 94 "../parser.act"

		assert(strlen(io_buffer) > 0);
		assert(!isspace((unsigned char) io_buffer[0]));
		rtrim(io_buffer);
		ZIn = xstrdup(io_buffer);
		io_flush();
	
#line 454 "bnf_parser.c"
			}
			/* END OF EXTRACT: name */
			break;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
		/* BEGINNING OF INLINE: 43 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (tok_equals):
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
#line 218 "../parser.act"

		expected("production assignment");
	
#line 482 "bnf_parser.c"
				}
				/* END OF ACTION: err-expected-equals */
			}
		ZL2:;
		}
		/* END OF INLINE: 43 */
		prod_list_Hof_Halts (&ZIa);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
		/* BEGINNING OF ACTION: make-production */
		{
#line 150 "../parser.act"

		(ZIp) = xmalloc(sizeof *(ZIp));
		(ZIp)->name = (ZIn);
		(ZIp)->alts = (ZIa);
		(ZIp)->next = NULL;
	
#line 503 "bnf_parser.c"
		}
		/* END OF ACTION: make-production */
		/* BEGINNING OF INLINE: 44 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (tok_sep):
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
#line 214 "../parser.act"

		expected("alternative separator");
	
#line 526 "bnf_parser.c"
				}
				/* END OF ACTION: err-expected-sep */
			}
		ZL4:;
		}
		/* END OF INLINE: 44 */
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOp = ZIp;
}

void
prod_bnf_Hgrammar(map_production *ZOl)
{
	map_production ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_list_Hof_Hproductions (&ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	{
		/* BEGINNING OF ACTION: make-empty-production */
		{
#line 157 "../parser.act"

		(ZIl) = NULL;
	
#line 566 "bnf_parser.c"
		}
		/* END OF ACTION: make-empty-production */
		/* BEGINNING OF ACTION: err-unhandled */
		{
#line 222 "../parser.act"

		expected(NULL);
	
#line 575 "bnf_parser.c"
		}
		/* END OF ACTION: err-unhandled */
	}
ZL0:;
	*ZOl = ZIl;
}

/* BEGINNING OF TRAILER */

#line 225 "../parser.act"

#line 587 "bnf_parser.c"

/* END OF FILE */
