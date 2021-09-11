/*
 * Automatically generated from the files:
 *	src/abnf/parser.sid
 * and
 *	src/parser.act
 * by:
 *	sid
 */

/* BEGINNING OF HEADER */

#line 32 "src/parser.act"


	#include <assert.h>
	#include <limits.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <errno.h>
	#include <ctype.h>

	#include "../parsing_error.h"
	#include "../txt.h"
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

	/* XXX: workarounds for SID's identifier escaping */
	#define prod_iso_Hebnf FORM_ENTRY
	#define TOK_CI__LITERAL TOK_CI_LITERAL
	#define TOK_CS__LITERAL TOK_CS_LITERAL

	#include "parser.h"
	#include "lexer.h"

	#include "io.h"

	typedef char         map_char;
	typedef const char * map_string;
	typedef struct txt   map_txt;
	typedef unsigned int map_count;

	typedef struct ast_term * map_term;
	typedef struct ast_alt * map_alt;

	struct act_state {
		enum LX_TOKEN lex_tok;
		enum LX_TOKEN lex_tok_save;
		int invisible;
	};

	struct lex_state {
		struct LX_STATE lx;
		struct lx_dynbuf buf;

		int (*f)(void *opaque);
		void *opaque;

		/* TODO: use lx's generated conveniences for the pattern buffer */
		char a[512];
		char *p;

		parsing_error_queue errors;
	};

	#define CURRENT_TERMINAL (act_state->lex_tok)
	#define ERROR_TERMINAL   (TOK_ERROR)
	#define ADVANCE_LEXER    do { act_state->lex_tok = LX_NEXT(&lex_state->lx); } while (0)
	#define SAVE_LEXER(tok)  do { act_state->lex_tok_save = act_state->lex_tok; \
	                              act_state->lex_tok = tok;                     } while (0)
	#define RESTORE_LEXER    do { act_state->lex_tok = act_state->lex_tok_save; } while (0)

	extern int allow_undefined;

	static const char *
	prefix(int base)
	{
		switch (base) {
		case 16: return "%x";
		case 10: return "%d";
		case  8: return "%o";
		case  2: return "%b";
		default: return "";
		}
	}

	static int
	string(const char *p, struct txt *t, int base)
	{
		char *q;

		assert(p != NULL);
		assert(t != NULL);
		assert(t->p != NULL);
		assert(base > 0);

		{
			const char *s;
			size_t z;

			s = prefix(base);
			z = strlen(s);

			assert(0 == strncmp(p, s, z));

			p += z;
		}

		q = (void *) t->p;

		for (;;) {
			unsigned long n;
			char *e;

			n = strtoul(p, &e, base);
			if (n == ULONG_MAX) {
				return -1;
			}

			if (n > UCHAR_MAX) {
				errno = ERANGE;
				return -1;
			}

			*q++ = (unsigned char) n;

			if (*e == '\0') {
				break;
			}

			assert(*e == '.');

			p = e + 1;
		}

		t->n = q - t->p;

		return 0;
	}

	static int
	range(const char *p,
		unsigned char *a, unsigned char *b,
		int base)
	{
		unsigned long m, n;
		char *e;

		assert(p != NULL);
		assert(a != NULL);
		assert(b != NULL);
		assert(base > 0);

		{
			const char *s;
			size_t z;

			s = prefix(base);
			z = strlen(s);

			assert(0 == strncmp(p, s, z));

			p += z;
		}

		m = strtoul(p, &e, base);
		if (m == ULONG_MAX) {
			return -1;
		}

		p = e;

		assert(*p == '-');
		p++;

		n = strtoul(p, &e, base);
		if (n == ULONG_MAX) {
			return -1;
		}

		assert(*e == '\0');

		if (m > UCHAR_MAX || n > UCHAR_MAX) {
			errno = ERANGE;
			return -1;
		}

		*a = m;
		*b = n;

		return 0;
	}

	static const char *
	ltrim(const char *s)
	{
		while (isspace((unsigned char) *s)) {
			s++;
		}

		return s;
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

	static const char *
	trim(char *s)
	{
		rtrim(s);
		return ltrim(s);
	}

	static void
	err(struct lex_state *lex_state, const char *fmt, ...)
	{
		parsing_error error;
		va_list ap;

		assert(lex_state != NULL);

		error.line = lex_state->lx.start.line;
		error.col  = lex_state->lx.start.col;

		va_start(ap, fmt);
		vsnprintf(error.description, PARSING_ERROR_DESCRIPTION_SIZE, fmt, ap);
		va_end(ap);

		parsing_error_queue_push(&(lex_state->errors), error);
	}

	static void
	err_expected(struct lex_state *lex_state, const char *token)
	{
		err(lex_state, "Syntax error: expected %s", token);
	}

	static void
	err_unimplemented(struct lex_state *lex_state, const char *s)
	{
		err(lex_state, "Unimplemented: %s", s);
	}

	static const char *
	pattern_buffer(struct lex_state *lex_state)
	{
		const char *s;

		assert(lex_state != NULL);

		/* TODO */
		*lex_state->p++ = '\0';

		s = xstrdup(lex_state->a);
		if (s == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		lex_state->p = lex_state->a;

		return s;
	}

#line 293 "src/abnf/parser.c"


#ifndef ERROR_TERMINAL
#error "-s no-numeric-terminals given and ERROR_TERMINAL is not defined"
#endif

/* BEGINNING OF FUNCTION DECLARATIONS */

static void prod_factor(lex_state, act_state, map_term *);
extern void prod_abnf(lex_state, act_state, map_rule *);
static void prod_list_Hof_Hterms(lex_state, act_state, map_term *);
static void prod_list_Hof_Hrules(lex_state, act_state, map_rule *);
static void prod_list_Hof_Halts(lex_state, act_state, map_alt *);
static void prod_body(lex_state, act_state);
static void prod_term(lex_state, act_state, map_term *);
static void prod_rule(lex_state, act_state, map_rule *);
static void prod_87(lex_state, act_state, map_count *);
static void prod_101(lex_state, act_state, map_rule *);
static void prod_102(lex_state, act_state, map_term *, map_alt *);
static void prod_103(lex_state, act_state, map_term *);
static void prod_factor_C_Celement(lex_state, act_state, map_term *);
static void prod_105(lex_state, act_state, map_count *, map_term *);
static void prod_106(lex_state, act_state, map_term *);

/* BEGINNING OF STATIC VARIABLES */


/* BEGINNING OF FUNCTION DEFINITIONS */

static void
prod_factor(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_COUNT):
		{
			map_count ZI104;

			/* BEGINNING OF EXTRACT: COUNT */
			{
#line 363 "src/parser.act"

		ZI104 = strtoul(lex_state->buf.a, NULL, 10);
		/* TODO: range check */
	
#line 340 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: COUNT */
			ADVANCE_LEXER;
			prod_105 (lex_state, act_state, &ZI104, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
		}
		break;
	case (TOK_STARTOPT):
		{
			map_alt ZIa;
			map_count ZImin;
			map_count ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: rep-zero-or-one */
			{
#line 544 "src/parser.act"

		(ZImin) = 0;
		(ZImax) = 1;

		/* workaround for SID's ! = f(); */
		(void) (ZImin);
		(void) (ZImax);
	
#line 374 "src/abnf/parser.c"
			}
			/* END OF ACTION: rep-zero-or-one */
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDOPT):
				break;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 641 "src/parser.act"

		(ZIt) = ast_make_group_term(act_state->invisible, (ZIa));
	
#line 390 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 553 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 402 "src/abnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_STARTGROUP): case (TOK_CHAR): case (TOK_IDENT): case (TOK_CI__LITERAL):
	case (TOK_CS__LITERAL): case (TOK_PROSE): case (TOK_BINSTR): case (TOK_DECSTR):
	case (TOK_HEXSTR): case (TOK_BINRANGE): case (TOK_DECRANGE): case (TOK_HEXRANGE):
		{
			prod_factor_C_Celement (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
		}
		break;
	case (TOK_REP):
		{
			map_count ZImin;
			map_count ZI86;
			map_count ZImax;

			/* BEGINNING OF ACTION: rep-zero-or-more */
			{
#line 535 "src/parser.act"

		(ZImin) = 0;
		(ZI86) = 0;

		/* workaround for SID's ! = f(); */
		(void) (ZImin);
		(void) (ZI86);
	
#line 435 "src/abnf/parser.c"
			}
			/* END OF ACTION: rep-zero-or-more */
			ADVANCE_LEXER;
			prod_87 (lex_state, act_state, &ZImax);
			prod_factor_C_Celement (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 553 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 454 "src/abnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
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

void
prod_abnf(lex_state lex_state, act_state act_state, map_rule *ZOl)
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
#line 674 "src/parser.act"

		(ZIl) = NULL;
	
#line 502 "src/abnf/parser.c"
		}
		/* END OF ACTION: make-empty-rule */
		/* BEGINNING OF ACTION: err-syntax */
		{
#line 717 "src/parser.act"

		err(lex_state, "Syntax error");
		exit(EXIT_FAILURE);
	
#line 512 "src/abnf/parser.c"
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
		prod_factor (lex_state, act_state, &ZIl);
		prod_103 (lex_state, act_state, &ZIl);
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
		prod_101 (lex_state, act_state, &ZIl);
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
		prod_102 (lex_state, act_state, &ZIt, &ZIl);
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

			/* BEGINNING OF INLINE: 75 */
			{
				{
					switch (CURRENT_TERMINAL) {
					case (TOK_CHAR):
						/* BEGINNING OF EXTRACT: CHAR */
						{
#line 340 "src/parser.act"

		assert(strlen(lex_state->buf.a) == 1);

		ZIc = lex_state->buf.a[0];
	
#line 616 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: CHAR */
						break;
					default:
						goto ZL1;
					}
					ADVANCE_LEXER;
				}
			}
			/* END OF INLINE: 75 */
			/* BEGINNING OF ACTION: pattern-char */
			{
#line 520 "src/parser.act"

		/* TODO */
		*lex_state->p++ = (ZIc);
	
#line 634 "src/abnf/parser.c"
			}
			/* END OF ACTION: pattern-char */
			/* BEGINNING OF INLINE: body */
			goto ZL2_body;
			/* END OF INLINE: body */
		}
		/* UNREACHED */
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
			map_string ZIx;

			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 346 "src/parser.act"

		/*
		 * This rtrim() is for EBNF, which would require n-token lookahead
		 * in order to lex just an ident (as ident may contain whitespace).
		 *
		 * I'm trimming here (for all grammars) because it's simpler than
		 * doing this for just EBNF specifically, and harmless to others.
		 */
		rtrim(lex_state->buf.a);

		ZIx = xstrdup(lex_state->buf.a);
		if (ZIx == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 682 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-rule-term */
			{
#line 584 "src/parser.act"

		struct ast_rule *r;

		/*
		 * Regardless of whether a rule exists (yet) by this name, we make
		 * a placeholder rule just so that we have an ast_rule struct
		 * at which to point. This saves passing the grammar around, which
		 * keeps the rule-building productions simpler.
		 */
		r = ast_make_rule((ZIx), NULL);
		if (r == NULL) {
			perror("ast_make_rule");
			goto ZL1;
		}

		(ZIt) = ast_make_rule_term(act_state->invisible, r);
	
#line 706 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-rule-term */
		}
		break;
	case (TOK_CHAR): case (TOK_CI__LITERAL): case (TOK_CS__LITERAL): case (TOK_PROSE):
		{
			prod_body (lex_state, act_state);
			prod_106 (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
		}
		break;
	case (TOK_BINRANGE): case (TOK_DECRANGE): case (TOK_HEXRANGE):
		{
			map_char ZIm;
			map_char ZIn;

			/* BEGINNING OF INLINE: escrange */
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_BINRANGE):
					{
						/* BEGINNING OF EXTRACT: BINRANGE */
						{
#line 468 "src/parser.act"

		if (-1 == range(lex_state->buf.a, (unsigned char *) &ZIm, (unsigned char *) &ZIn,  2)) {
			if (errno == ERANGE) {
				err(lex_state, "hex sequence %s out of range: expected %s0..%s%x inclusive",
					lex_state->buf.a, prefix(2), prefix(2), UCHAR_MAX);
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}
	
#line 745 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: BINRANGE */
						ADVANCE_LEXER;
					}
					break;
				case (TOK_DECRANGE):
					{
						/* BEGINNING OF EXTRACT: DECRANGE */
						{
#line 492 "src/parser.act"

		if (-1 == range(lex_state->buf.a, (unsigned char *) &ZIm, (unsigned char *) &ZIn, 10)) {
			if (errno == ERANGE) {
				err(lex_state, "decimal sequence %s out of range: expected %s0..%s%u inclusive",
					lex_state->buf.a, prefix(10), prefix(10), UCHAR_MAX);
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}
	
#line 767 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: DECRANGE */
						ADVANCE_LEXER;
					}
					break;
				case (TOK_HEXRANGE):
					{
						/* BEGINNING OF EXTRACT: HEXRANGE */
						{
#line 504 "src/parser.act"

		if (-1 == range(lex_state->buf.a, (unsigned char *) &ZIm, (unsigned char *) &ZIn, 16)) {
			if (errno == ERANGE) {
				err(lex_state, "hex sequence %s out of range: expected %s0..%s%x inclusive",
					lex_state->buf.a, prefix(16), prefix(16), UCHAR_MAX);
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}
	
#line 789 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: HEXRANGE */
						ADVANCE_LEXER;
					}
					break;
				default:
					goto ZL1;
				}
			}
			/* END OF INLINE: escrange */
			/* BEGINNING OF ACTION: make-range-term */
			{
#line 645 "src/parser.act"

		struct ast_alt *a;
		int i;

		a = NULL;

		for (i = (unsigned char) (ZIm); i <= (unsigned char) (ZIn); i++) {
			struct ast_alt *new;
			struct ast_term *t;

			t = ast_make_char_term(act_state->invisible, i);
			new = ast_make_alt(act_state->invisible, t);

			new->next = a;
			a = new;
		}

		(ZIt) = ast_make_group_term(act_state->invisible, a);
	
#line 822 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-range-term */
		}
		break;
	case (TOK_BINSTR): case (TOK_DECSTR): case (TOK_HEXSTR):
		{
			map_txt ZIx;

			/* BEGINNING OF INLINE: escstr */
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_BINSTR):
					{
						/* BEGINNING OF EXTRACT: BINSTR */
						{
#line 388 "src/parser.act"

		ZIx.p = xstrdup(lex_state->buf.a);
		if (ZIx.p == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		if (-1 == string(lex_state->buf.a, &ZIx,  2)) {
			if (errno == ERANGE) {
				err(lex_state, "binary sequence %s out of range: expected %s0..%s%s inclusive",
					lex_state->buf.a, prefix(2), prefix(2), "11111111");
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}

		/* TODO: free */
	
#line 858 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: BINSTR */
						ADVANCE_LEXER;
					}
					break;
				case (TOK_DECSTR):
					{
						/* BEGINNING OF EXTRACT: DECSTR */
						{
#line 428 "src/parser.act"

		ZIx.p = xstrdup(lex_state->buf.a);
		if (ZIx.p == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		if (-1 == string(lex_state->buf.a, &ZIx, 10)) {
			if (errno == ERANGE) {
				err(lex_state, "decimal sequence %s out of range: expected %s0..%s%u inclusive",
					lex_state->buf.a, prefix(10), prefix(10), UCHAR_MAX);
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}

		/* TODO: free */
	
#line 888 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: DECSTR */
						ADVANCE_LEXER;
					}
					break;
				case (TOK_HEXSTR):
					{
						/* BEGINNING OF EXTRACT: HEXSTR */
						{
#line 448 "src/parser.act"

		ZIx.p = xstrdup(lex_state->buf.a);
		if (ZIx.p == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		if (-1 == string(lex_state->buf.a, &ZIx, 16)) {
			if (errno == ERANGE) {
				err(lex_state, "hex sequence %s out of range: expected %s0..%s%x inclusive",
					lex_state->buf.a, prefix(16), prefix(16), UCHAR_MAX);
			} else {
				err(lex_state, "%s: %s", lex_state->buf.a, strerror(errno));
			}
			goto ZL1;
		}

		/* TODO: free */
	
#line 918 "src/abnf/parser.c"
						}
						/* END OF EXTRACT: HEXSTR */
						ADVANCE_LEXER;
					}
					break;
				default:
					goto ZL1;
				}
			}
			/* END OF INLINE: escstr */
			/* BEGINNING OF ACTION: make-cs-literal-term */
			{
#line 613 "src/parser.act"

		(ZIt) = ast_make_literal_term(act_state->invisible, &(ZIx), 0);
	
#line 935 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-cs-literal-term */
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
prod_rule(lex_state lex_state, act_state act_state, map_rule *ZOr)
{
	map_rule ZIr;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_string ZIs;

		switch (CURRENT_TERMINAL) {
		case (TOK_IDENT):
			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 346 "src/parser.act"

		/*
		 * This rtrim() is for EBNF, which would require n-token lookahead
		 * in order to lex just an ident (as ident may contain whitespace).
		 *
		 * I'm trimming here (for all grammars) because it's simpler than
		 * doing this for just EBNF specifically, and harmless to others.
		 */
		rtrim(lex_state->buf.a);

		ZIs = xstrdup(lex_state->buf.a);
		if (ZIs == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 985 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			break;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
		/* BEGINNING OF INLINE: 95 */
		{
			switch (CURRENT_TERMINAL) {
			case (TOK_EQUALS):
				{
					map_alt ZIa;

					/* BEGINNING OF INLINE: 96 */
					{
						{
							switch (CURRENT_TERMINAL) {
							case (TOK_EQUALS):
								break;
							default:
								goto ZL4;
							}
							ADVANCE_LEXER;
						}
						goto ZL3;
					ZL4:;
						{
							/* BEGINNING OF ACTION: err-expected-equals */
							{
#line 730 "src/parser.act"

		err_expected(lex_state, "production rule assignment");
	
#line 1020 "src/abnf/parser.c"
							}
							/* END OF ACTION: err-expected-equals */
						}
					ZL3:;
					}
					/* END OF INLINE: 96 */
					prod_list_Hof_Halts (lex_state, act_state, &ZIa);
					if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
						RESTORE_LEXER;
						goto ZL1;
					}
					/* BEGINNING OF ACTION: make-rule */
					{
#line 670 "src/parser.act"

		(ZIr) = ast_make_rule((ZIs), (ZIa));
	
#line 1038 "src/abnf/parser.c"
					}
					/* END OF ACTION: make-rule */
				}
				break;
			case (TOK_ALTINC):
				{
					map_rule ZIl;
					map_alt ZIa;

					/* BEGINNING OF INLINE: 97 */
					{
						{
							switch (CURRENT_TERMINAL) {
							case (TOK_ALTINC):
								break;
							default:
								goto ZL6;
							}
							ADVANCE_LEXER;
						}
						goto ZL5;
					ZL6:;
						{
							/* BEGINNING OF ACTION: err-expected-equals */
							{
#line 730 "src/parser.act"

		err_expected(lex_state, "production rule assignment");
	
#line 1068 "src/abnf/parser.c"
							}
							/* END OF ACTION: err-expected-equals */
						}
					ZL5:;
					}
					/* END OF INLINE: 97 */
					/* BEGINNING OF ACTION: current-rules */
					{
#line 699 "src/parser.act"

    err(lex_state, "unimplemented");
		(ZIl) = NULL;
		goto ZL1;
	
#line 1083 "src/abnf/parser.c"
					}
					/* END OF ACTION: current-rules */
					prod_list_Hof_Halts (lex_state, act_state, &ZIa);
					if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
						RESTORE_LEXER;
						goto ZL1;
					}
					/* BEGINNING OF ACTION: find-rule */
					{
#line 705 "src/parser.act"

		assert((ZIs) != NULL);

		(ZIr) = ast_find_rule((ZIl), (ZIs));
	
#line 1099 "src/abnf/parser.c"
					}
					/* END OF ACTION: find-rule */
					/* BEGINNING OF ACTION: add-alts */
					{
#line 711 "src/parser.act"

		err(lex_state, "Unimplemented");
		goto ZL1;
	
#line 1109 "src/abnf/parser.c"
					}
					/* END OF ACTION: add-alts */
				}
				break;
			default:
				goto ZL1;
			}
		}
		/* END OF INLINE: 95 */
		/* BEGINNING OF INLINE: 98 */
		{
			switch (CURRENT_TERMINAL) {
			case (TOK_EOF):
				{
					ADVANCE_LEXER;
				}
				break;
			case (TOK_SEP):
				{
					ADVANCE_LEXER;
				}
				break;
			default:
				goto ZL8;
			}
			goto ZL7;
		ZL8:;
			{
				/* BEGINNING OF ACTION: err-expected-sep */
				{
#line 726 "src/parser.act"

		err_expected(lex_state, "production rule separator");
	
#line 1144 "src/abnf/parser.c"
				}
				/* END OF ACTION: err-expected-sep */
			}
		ZL7:;
		}
		/* END OF INLINE: 98 */
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOr = ZIr;
}

static void
prod_87(lex_state lex_state, act_state act_state, map_count *ZOmax)
{
	map_count ZImax;

	switch (CURRENT_TERMINAL) {
	case (TOK_COUNT):
		{
			/* BEGINNING OF EXTRACT: COUNT */
			{
#line 363 "src/parser.act"

		ZImax = strtoul(lex_state->buf.a, NULL, 10);
		/* TODO: range check */
	
#line 1175 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: COUNT */
			ADVANCE_LEXER;
		}
		break;
	default:
		{
			map_count ZI89;

			/* BEGINNING OF ACTION: rep-zero-or-more */
			{
#line 535 "src/parser.act"

		(ZI89) = 0;
		(ZImax) = 0;

		/* workaround for SID's ! = f(); */
		(void) (ZI89);
		(void) (ZImax);
	
#line 1196 "src/abnf/parser.c"
			}
			/* END OF ACTION: rep-zero-or-more */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	*ZOmax = ZImax;
}

static void
prod_101(lex_state lex_state, act_state act_state, map_rule *ZIl)
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
#line 689 "src/parser.act"

		if (ast_find_rule((ZIr), (*ZIl)->name)) {
      err(lex_state, "production rule <%s> already exists", (*ZIl)->name);
			return;
		}

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIr);
	
#line 1232 "src/abnf/parser.c"
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
prod_102(lex_state lex_state, act_state act_state, map_term *ZIt, map_alt *ZOl)
{
	map_alt ZIl;

	switch (CURRENT_TERMINAL) {
	case (TOK_ALT):
		{
			map_alt ZIa;

			/* BEGINNING OF INLINE: 92 */
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
#line 722 "src/parser.act"

		err_expected(lex_state, "alternative separator");
	
#line 1278 "src/abnf/parser.c"
					}
					/* END OF ACTION: err-expected-alt */
				}
			ZL2:;
			}
			/* END OF INLINE: 92 */
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: make-alt */
			{
#line 666 "src/parser.act"

		(ZIl) = ast_make_alt(act_state->invisible, (*ZIt));
	
#line 1296 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-alt */
			/* BEGINNING OF ACTION: add-alt-to-list */
			{
#line 684 "src/parser.act"

		assert((ZIl)->next == NULL);
		(ZIl)->next = (ZIa);
	
#line 1306 "src/abnf/parser.c"
			}
			/* END OF ACTION: add-alt-to-list */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-alt */
			{
#line 666 "src/parser.act"

		(ZIl) = ast_make_alt(act_state->invisible, (*ZIt));
	
#line 1319 "src/abnf/parser.c"
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
prod_103(lex_state lex_state, act_state act_state, map_term *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_REP): case (TOK_STARTGROUP): case (TOK_STARTOPT): case (TOK_CHAR):
	case (TOK_IDENT): case (TOK_COUNT): case (TOK_CI__LITERAL): case (TOK_CS__LITERAL):
	case (TOK_PROSE): case (TOK_BINSTR): case (TOK_DECSTR): case (TOK_HEXSTR):
	case (TOK_BINRANGE): case (TOK_DECRANGE): case (TOK_HEXRANGE):
		{
			map_term ZIt;

			prod_list_Hof_Hterms (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-term-to-list */
			{
#line 679 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIt);
	
#line 1358 "src/abnf/parser.c"
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
prod_factor_C_Celement(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_STARTGROUP):
		{
			map_alt ZIa;

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
#line 641 "src/parser.act"

		(ZIt) = ast_make_group_term(act_state->invisible, (ZIa));
	
#line 1402 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-group-term */
		}
		break;
	case (TOK_CHAR): case (TOK_IDENT): case (TOK_CI__LITERAL): case (TOK_CS__LITERAL):
	case (TOK_PROSE): case (TOK_BINSTR): case (TOK_DECSTR): case (TOK_HEXSTR):
	case (TOK_BINRANGE): case (TOK_DECRANGE): case (TOK_HEXRANGE):
		{
			prod_term (lex_state, act_state, &ZIt);
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
prod_105(lex_state lex_state, act_state act_state, map_count *ZI104, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_REP):
		{
			map_count ZImax;

			ADVANCE_LEXER;
			prod_87 (lex_state, act_state, &ZImax);
			prod_factor_C_Celement (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 553 "src/parser.act"

		assert((ZImax) >= (*ZI104) || !(ZImax));

		(ZIt)->min = (*ZI104);
		(ZIt)->max = (ZImax);
	
#line 1457 "src/abnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_STARTGROUP): case (TOK_CHAR): case (TOK_IDENT): case (TOK_CI__LITERAL):
	case (TOK_CS__LITERAL): case (TOK_PROSE): case (TOK_BINSTR): case (TOK_DECSTR):
	case (TOK_HEXSTR): case (TOK_BINRANGE): case (TOK_DECRANGE): case (TOK_HEXRANGE):
		{
			prod_factor_C_Celement (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 553 "src/parser.act"

		assert((*ZI104) >= (*ZI104) || !(*ZI104));

		(ZIt)->min = (*ZI104);
		(ZIt)->max = (*ZI104);
	
#line 1480 "src/abnf/parser.c"
			}
			/* END OF ACTION: set-repeat */
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
prod_106(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_CI__LITERAL):
		{
			map_txt ZIx;

			/* BEGINNING OF EXTRACT: CI_LITERAL */
			{
#line 373 "src/parser.act"

		ZIx.p = pattern_buffer(lex_state);
		ZIx.n = strlen(ZIx.p);
	
#line 1515 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: CI_LITERAL */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-ci-literal-term */
			{
#line 602 "src/parser.act"

		size_t i;

		/* normalise case-insensitive strings for aesthetic reasons only */
		for (i = 0; i < (ZIx).n; i++) {
			((char *) (ZIx).p)[i] = tolower((unsigned char) (ZIx).p[i]);
		}

		(ZIt) = ast_make_literal_term(act_state->invisible, &(ZIx), 1);
	
#line 1532 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-ci-literal-term */
		}
		break;
	case (TOK_CS__LITERAL):
		{
			map_txt ZIx;

			/* BEGINNING OF EXTRACT: CS_LITERAL */
			{
#line 378 "src/parser.act"

		ZIx.p = pattern_buffer(lex_state);
		ZIx.n = strlen(ZIx.p);
	
#line 1548 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: CS_LITERAL */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-cs-literal-term */
			{
#line 613 "src/parser.act"

		(ZIt) = ast_make_literal_term(act_state->invisible, &(ZIx), 0);
	
#line 1558 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-cs-literal-term */
		}
		break;
	case (TOK_PROSE):
		{
			map_string ZIs;

			/* BEGINNING OF EXTRACT: PROSE */
			{
#line 383 "src/parser.act"

		ZIs = pattern_buffer(lex_state);
	
#line 1573 "src/abnf/parser.c"
			}
			/* END OF EXTRACT: PROSE */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-prose-term */
			{
#line 621 "src/parser.act"

		const char *s;

		s = xstrdup(trim((char *) (ZIs)));

		free((void *) (ZIs));

		if (!strcmp(s, "kgt:invisible")) {
			act_state->invisible = 1;

			(ZIt) = ast_make_empty_term(act_state->invisible);
		} else if (!strcmp(s, "kgt:visible")) {
			act_state->invisible = 0;

			(ZIt) = ast_make_empty_term(act_state->invisible);
		} else {
			(ZIt) = ast_make_prose_term(act_state->invisible, s);
		}
	
#line 1599 "src/abnf/parser.c"
			}
			/* END OF ACTION: make-prose-term */
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

/* BEGINNING OF TRAILER */

#line 738 "src/parser.act"


	static int
	lgetc(struct LX_STATE *lx)
	{
		const struct lex_state *lex_state;

		assert(lx != NULL);
		assert(lx->getc_opaque != NULL);

		lex_state = lx->getc_opaque;

		assert(lex_state->f != NULL);

		return lex_state->f(lex_state->opaque);
	}

	struct ast_rule *
	FORM_INPUT(int (*f)(void *opaque), void *opaque, parsing_error_queue* errors)
	{
		struct act_state  act_state_s;
		struct act_state *act_state;
		struct lex_state  lex_state_s;
		struct lex_state *lex_state;

		struct LX_STATE *lx;
		struct ast_rule *g;

		/* for dialects which don't use these */
		(void) string;
		(void) range;
		(void) ltrim;
		(void) rtrim;
		(void) trim;
		(void) err_unimplemented;

		assert(f != NULL);

		g = NULL;

		lex_state    = &lex_state_s;
		lex_state->p = lex_state->a;
		lex_state->errors = NULL;

		lx = &lex_state->lx;

		LX_INIT(lx);

		lx->lgetc       = lgetc;
		lx->getc_opaque = lex_state;

		lex_state->f       = f;
		lex_state->opaque  = opaque;

		lex_state->buf.a   = NULL;
		lex_state->buf.len = 0;

		/* XXX: unneccessary since we're lexing from a string */
		lx->buf_opaque = &lex_state->buf;
		lx->push       = CAT(LX_PREFIX, _dynpush);
		lx->clear      = CAT(LX_PREFIX, _dynclear);
		lx->free       = CAT(LX_PREFIX, _dynfree);

		/* XXX */
		lx->free = NULL;

		/* This is a workaround for ADVANCE_LEXER assuming a pointer */
		act_state = &act_state_s;

		act_state->invisible = 0;

		ADVANCE_LEXER;
		FORM_ENTRY(lex_state, act_state, &g);

		/* TODO: handle error */

		/* substitute placeholder rules for the real thing */
		{
			const struct ast_rule *p;
			const struct ast_alt *q;
			struct ast_term *t;
			struct ast_rule *r;

			for (p = g; p != NULL; p = p->next) {
				for (q = p->alts; q != NULL; q = q->next) {
					for (t = q->terms; t != NULL; t = t->next) {
						if (t->type != TYPE_RULE) {
							continue;
						}

						r = ast_find_rule(g, t->u.rule->name);
						if (r != NULL) {
							free((char *) t->u.rule->name);
							ast_free_rule((void *) t->u.rule);
							t->u.rule = r;
							continue;
						}

						if (!allow_undefined) {
							err(lex_state, "production rule <%s> not defined", t->u.rule->name);
							/* XXX: would leak the ast_rule here */
							continue;
						}

						{
							const char *token;

							token = t->u.rule->name;

							ast_free_rule((void *) t->u.rule);

							t->type    = TYPE_TOKEN;
							t->u.token = token;
						}
					}
				}
			}
		}

		*errors = lex_state->errors;
		return g;
	}

#line 1743 "src/abnf/parser.c"

/* END OF FILE */
