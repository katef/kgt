# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = trd_output.o

REMOVE = rm -f

trd.a: $(TARGETS)
	$(AR) rc trd.a $(TARGETS)
	$(RANLIB) trd.a

clean:
	$(REMOVE) trd.a $(TARGETS)

