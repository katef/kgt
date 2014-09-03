/*
 * Automatically generated from the files:
 *	src/wsn/parser.sid
 * and
 *	src/parser.act
 * by:
 *	sid
 */

/* BEGINNING OF HEADER */

#line 40 "src/parser.act"


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
	#define ERROR_TERMINAL   (TOK_ERROR)
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

#line 69 "src/wsn/parser.c"


#ifndef ERROR_TERMINAL
#error "-s no-numeric-terminals given and ERROR_TERMINAL is not defined"
#endif

/* BEGINNING OF FUNCTION DECLARATIONS */

static void prod_factor(map_term *);
static void prod_list_Hof_Hterms(map_term *);
static void prod_list_Hof_Halts(map_alt *);
static void prod_list_Hof_Hproductions(map_production *);
static void prod_58(map_production *);
static void prod_59(map_term *, map_alt *);
static void prod_term(map_term *);
static void prod_60(map_term *);
static void prod_production(map_production *);
extern void prod_wsn_Hgrammar(map_production *);

/* BEGINNING OF STATIC VARIABLES */


/* BEGINNING OF FUNCTION DEFINITIONS */

static void
prod_factor(map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_START_HGROUP):
		{
			map_alt ZIa;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (&ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_END_HGROUP):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-alt-group */
			{
#line 178 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->u.group = xmalloc(sizeof *(ZIt)->u.group);
		(ZIt)->u.group->kleene = KLEENE_GROUP;
		(ZIt)->u.group->alts = (ZIa);
	
#line 126 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-alt-group */
		}
		break;
	case (TOK_START_HOPT):
		{
			map_alt ZIa;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (&ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_END_HOPT):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-opt-group */
			{
#line 170 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->u.group = xmalloc(sizeof *(ZIt)->u.group);
		(ZIt)->u.group->kleene = KLEENE_OPTIONAL;
		(ZIt)->u.group->alts = (ZIa);
	
#line 157 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-opt-group */
		}
		break;
	case (TOK_START_HSTAR):
		{
			map_alt ZIa;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (&ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_END_HSTAR):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-star-group */
			{
#line 162 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_GROUP;
		(ZIt)->u.group = xmalloc(sizeof *(ZIt)->u.group);
		(ZIt)->u.group->kleene = KLEENE_STAR;
		(ZIt)->u.group->alts = (ZIa);
	
#line 188 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-star-group */
		}
		break;
	case (TOK_EMPTY): case (TOK_NAME): case (TOK_LITERAL):
		{
			prod_term (&ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
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
prod_list_Hof_Hterms(map_term *ZOl)
{
	map_term ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_factor (&ZIl);
		prod_60 (&ZIl);
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
		prod_59 (&ZIt, &ZIl);
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
prod_list_Hof_Hproductions(map_production *ZOl)
{
	map_production ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_production (&ZIl);
		prod_58 (&ZIl);
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
prod_58(map_production *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_NAME):
		{
			map_production ZIp;

			prod_list_Hof_Hproductions (&ZIp);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-production-to-list */
			{
#line 203 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIp);
	
#line 309 "src/wsn/parser.c"
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
prod_59(map_term *ZIt, map_alt *ZOl)
{
	map_alt ZIl;

	switch (CURRENT_TERMINAL) {
	case (TOK_ALT):
		{
			map_alt ZIa;

			/* BEGINNING OF INLINE: 50 */
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
#line 210 "src/parser.act"

		expected("alternative separator");
	
#line 355 "src/wsn/parser.c"
					}
					/* END OF ACTION: err-expected-alt */
				}
			ZL2:;
			}
			/* END OF INLINE: 50 */
			prod_list_Hof_Halts (&ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: make-alt */
			{
#line 144 "src/parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 375 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-alt */
			/* BEGINNING OF ACTION: add-alt-to-list */
			{
#line 198 "src/parser.act"

		assert((ZIl)->next == NULL);
		(ZIl)->next = (ZIa);
	
#line 385 "src/wsn/parser.c"
			}
			/* END OF ACTION: add-alt-to-list */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-alt */
			{
#line 144 "src/parser.act"

		(ZIl) = xmalloc(sizeof *(ZIl));
		(ZIl)->terms = (*ZIt);
		(ZIl)->next = NULL;
	
#line 400 "src/wsn/parser.c"
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
prod_term(map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_EMPTY):
		{
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-empty-term */
			{
#line 121 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_EMPTY;
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 434 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-empty-term */
		}
		break;
	case (TOK_LITERAL):
		{
			map_string ZIl;

			/* BEGINNING OF EXTRACT: LITERAL */
			{
#line 100 "src/parser.act"

		assert(strlen(io_buffer) > 0);
		ZIl = xstrdup(io_buffer);
		io_flush();
	
#line 451 "src/wsn/parser.c"
			}
			/* END OF EXTRACT: LITERAL */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-literal-term */
			{
#line 128 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_TERMINAL;	/* TODO rename to literal or vice-versa, perhaps */
		(ZIt)->u.literal = (ZIl);
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 465 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-literal-term */
		}
		break;
	case (TOK_NAME):
		{
			map_string ZIn;

			/* BEGINNING OF EXTRACT: NAME */
			{
#line 94 "src/parser.act"

		assert(strlen(io_buffer) > 0);
		assert(!isspace((unsigned char) io_buffer[0]));
		rtrim(io_buffer);
		ZIn = xstrdup(io_buffer);
		io_flush();
	
#line 484 "src/wsn/parser.c"
			}
			/* END OF EXTRACT: NAME */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-production-term */
			{
#line 136 "src/parser.act"

		(ZIt) = xmalloc(sizeof *(ZIt));
		(ZIt)->type = TYPE_PRODUCTION;
		(ZIt)->u.name = (ZIn);
		(ZIt)->repeat = 1;
		(ZIt)->next = NULL;
	
#line 498 "src/wsn/parser.c"
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
prod_60(map_term *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_START_HGROUP): case (TOK_START_HOPT): case (TOK_START_HSTAR): case (TOK_EMPTY):
	case (TOK_NAME): case (TOK_LITERAL):
		{
			map_term ZIt;

			prod_list_Hof_Hterms (&ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-term-to-list */
			{
#line 193 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIt);
	
#line 537 "src/wsn/parser.c"
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
		case (TOK_NAME):
			/* BEGINNING OF EXTRACT: NAME */
			{
#line 94 "src/parser.act"

		assert(strlen(io_buffer) > 0);
		assert(!isspace((unsigned char) io_buffer[0]));
		rtrim(io_buffer);
		ZIn = xstrdup(io_buffer);
		io_flush();
	
#line 577 "src/wsn/parser.c"
			}
			/* END OF EXTRACT: NAME */
			break;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
		/* BEGINNING OF INLINE: 53 */
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
#line 218 "src/parser.act"

		expected("production assignment");
	
#line 605 "src/wsn/parser.c"
				}
				/* END OF ACTION: err-expected-equals */
			}
		ZL2:;
		}
		/* END OF INLINE: 53 */
		prod_list_Hof_Halts (&ZIa);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
		/* BEGINNING OF ACTION: make-production */
		{
#line 150 "src/parser.act"

		(ZIp) = xmalloc(sizeof *(ZIp));
		(ZIp)->name = (ZIn);
		(ZIp)->alts = (ZIa);
		(ZIp)->next = NULL;
	
#line 626 "src/wsn/parser.c"
		}
		/* END OF ACTION: make-production */
		/* BEGINNING OF INLINE: 54 */
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
#line 214 "src/parser.act"

		expected("alternative separator");
	
#line 649 "src/wsn/parser.c"
				}
				/* END OF ACTION: err-expected-sep */
			}
		ZL4:;
		}
		/* END OF INLINE: 54 */
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOp = ZIp;
}

void
prod_wsn_Hgrammar(map_production *ZOl)
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
#line 157 "src/parser.act"

		(ZIl) = NULL;
	
#line 689 "src/wsn/parser.c"
		}
		/* END OF ACTION: make-empty-production */
		/* BEGINNING OF ACTION: err-unhandled */
		{
#line 222 "src/parser.act"

		expected(NULL);
	
#line 698 "src/wsn/parser.c"
		}
		/* END OF ACTION: err-unhandled */
	}
ZL0:;
	*ZOl = ZIl;
}

/* BEGINNING OF TRAILER */

#line 225 "src/parser.act"

#line 710 "src/wsn/parser.c"

/* END OF FILE */
