# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = sid_output.o

REMOVE = rm -f

sid.a: $(TARGETS)
	$(AR) rc sid.a $(TARGETS)
	$(RANLIB) sid.a

clean:
	$(REMOVE) sid.a $(TARGETS)

