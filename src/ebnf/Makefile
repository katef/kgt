# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = ebnf_lexer.o ebnf_parser.o ebnf_output.o

LEXI = lexi
LEXIFLAGS = -a -t tok_ -i lct_ -p ebnf_

SID = sid
SIDFLAGS = -s no-terminals -s no-numeric-terminals

REMOVE = rm -f

ebnf.a: $(TARGETS)
	$(AR) rc ebnf.a $(TARGETS)
	$(RANLIB) ebnf.a

ebnf_lexer.c ebnf_lexer.h: ebnf_lexer.lxi ../lexer.lct ebnf_parser.h
	$(LEXI) $(LEXIFLAGS) ebnf_lexer.lxi ../lexer.lct ebnf_lexer.c ebnf_lexer.h

ebnf_parser.c ebnf_parser.h: ebnf_parser.sid ../parser.act
	$(SID) $(SIDFLAGS) ebnf_parser.sid ../parser.act ebnf_parser.c ebnf_parser.h

clean:
	$(REMOVE) ebnf.a ebnf_lexer.c ebnf_lexer.h ebnf_parser.c ebnf_parser.h $(TARGETS)

