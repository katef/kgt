# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = bnf_lexer.o bnf_parser.o bnf_output.o

LEXI = lexi
LEXIFLAGS = -t tok_ -p bnf_

SID = sid
SIDFLAGS = -s no-terminals -s no-numeric-terminals

REMOVE = rm -f

bnf.a: $(TARGETS)
	$(AR) rc bnf.a $(TARGETS)
	$(RANLIB) bnf.a

bnf_lexer.o: bnf_lexer.h

bnf_lexer.c: bnf_lexer.lxi ../lexer.lct bnf_parser.h
	$(LEXI) $(LEXIFLAGS) -l c bnf_lexer.lxi ../lexer.lct bnf_lexer.c
	sed -i 's/LEXI_HEADER/"bnf_lexer.h"/' ${.TARGET}

bnf_lexer.h: bnf_lexer.lxi ../lexer.lct bnf_parser.h
	$(LEXI) $(LEXIFLAGS) -l h bnf_lexer.lxi ../lexer.lct bnf_lexer.h

bnf_parser.c bnf_parser.h: bnf_parser.sid ../parser.act
	$(SID) $(SIDFLAGS) bnf_parser.sid ../parser.act bnf_parser.c bnf_parser.h

clean:
	$(REMOVE) bnf.a bnf_lexer.c bnf_lexer.h bnf_parser.c bnf_parser.h $(TARGETS)

