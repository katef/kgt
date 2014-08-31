# $Id$

CC = tcc
CFLAGS = -Xp

TARGETS = dot_output.o

REMOVE = rm -f

dot.a: $(TARGETS)
	$(AR) rc dot.a $(TARGETS)
	$(RANLIB) dot.a

clean:
	$(REMOVE) dot.a $(TARGETS)

