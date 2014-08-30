# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = wsn_lexer.o wsn_parser.o wsn_output.o

LEXI = lexi
LEXIFLAGS = -t tok_ -p wsn_

SID = sid
SIDFLAGS = -s no-terminals -s no-numeric-terminals

REMOVE = rm -f

wsn.a: $(TARGETS)
	$(AR) rc wsn.a $(TARGETS)
	$(RANLIB) wsn.a

wsn_lexer.o: wsn_lexer.h

wsn_lexer.c: wsn_lexer.lxi ../lexer.lct wsn_parser.h
	$(LEXI) $(LEXIFLAGS) -l c wsn_lexer.lxi ../lexer.lct wsn_lexer.c
	sed -i 's/LEXI_HEADER/"wsn_lexer.h"/' ${.TARGET}

wsn_lexer.h: wsn_lexer.lxi ../lexer.lct wsn_parser.h
	$(LEXI) $(LEXIFLAGS) -l h wsn_lexer.lxi ../lexer.lct wsn_lexer.h

wsn_parser.c wsn_parser.h: wsn_parser.sid ../parser.act
	$(SID) $(SIDFLAGS) wsn_parser.sid ../parser.act wsn_parser.c wsn_parser.h

clean:
	$(REMOVE) wsn.a wsn_lexer.c wsn_lexer.h wsn_parser.c wsn_parser.h $(TARGETS)

