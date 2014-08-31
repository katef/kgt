# $Id: Makefile 18 2014-08-30 21:13:15Z kate $

CC = tcc
CFLAGS = -Xp

TARGETS = rrd_transform.o rrd_beautify.o rrd_output.o

REMOVE = rm -f

rrd.a: $(TARGETS)
	$(AR) rc rrd.a $(TARGETS)
	$(RANLIB) rrd.a

clean:
	$(REMOVE) rrd.a $(TARGETS)

