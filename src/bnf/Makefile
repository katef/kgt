# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = bnf_lexer.o bnf_parser.o bnf_output.o

LEXI = lexi
LEXIFLAGS = -a -t tok_ -i lct_ -p bnf_

SID = sid
SIDFLAGS = -s no-terminals -s no-numeric-terminals

REMOVE = rm -f

bnf.a: $(TARGETS)
	$(AR) rc bnf.a $(TARGETS)
	$(RANLIB) bnf.a

bnf_lexer.c bnf_lexer.h: bnf_lexer.lxi ../lexer.lct bnf_parser.h
	$(LEXI) $(LEXIFLAGS) bnf_lexer.lxi ../lexer.lct bnf_lexer.c bnf_lexer.h

bnf_parser.c bnf_parser.h: bnf_parser.sid ../parser.act
	$(SID) $(SIDFLAGS) bnf_parser.sid ../parser.act bnf_parser.c bnf_parser.h

clean:
	$(REMOVE) bnf.a bnf_lexer.c bnf_lexer.h bnf_parser.c bnf_parser.h $(TARGETS)

