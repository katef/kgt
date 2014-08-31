# $Id$

TARGETS = main.o io.o xalloc.o
ARCHIVES = bnf/bnf.a wsn/wsn.a ebnf/ebnf.a sid/sid.a trd/trd.a dot/dot.a

LEXI = lexi
LEXIFLAGS = -a

REMOVE = rm -f
CD = cd

CC = tcc
CFLAGS = -Xp -Ysystem	# TODO: -Yposix

kgt: $(TARGETS) $(ARCHIVES)
	$(CC) $(CFLAGS) -o kgt $(TARGETS) $(ARCHIVES)

bnf/bnf.a: bnf/bnf_lexer.lxi bnf/bnf_parser.sid parser.act
	$(CD) bnf && $(MAKE)

wsn/wsn.a: wsn/wsn_lexer.lxi wsn/wsn_parser.sid parser.act
	$(CD) wsn && $(MAKE)

ebnf/ebnf.a: ebnf/ebnf_lexer.lxi ebnf/ebnf_parser.sid parser.act
	$(CD) ebnf && $(MAKE)

sid/sid.a:
	$(CD) sid && $(MAKE)

trd/trd.a:
	$(CD) trd && $(MAKE)

dot/dot.a:
	$(CD) dot && $(MAKE)

main.c: $(ARCHIVES)

clean:
	$(REMOVE) kgt $(TARGETS)
	$(CD) bnf && $(MAKE) clean
	$(CD) wsn && $(MAKE) clean
	$(CD) ebnf && $(MAKE) clean
	$(CD) sid && $(MAKE) clean
	$(CD) trd && $(MAKE) clean
	$(CD) dot && $(MAKE) clean

