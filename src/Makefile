.include "../share/mk/top.mk"

SRC += src/main.c
SRC += src/xalloc.c

PROG += kgt

# XXX: ${SRC:Msrc/*.c} matches src/xyz/*.c which is not what we want
#.for src in ${SRC:Msrc/*.c}
#${BUILD}/bin/kgt: ${BUILD}/${src:R}.o
#.endfor

${BUILD}/bin/kgt: ${BUILD}/src/main.o
${BUILD}/bin/kgt: ${BUILD}/src/xalloc.o

.for part in ${PART:Mbnf} ${PART:Mebnf} ${PART:Msid} ${PART:Mtrd} \
	${PART:Mdot} ${PART:Mrrd} ${PART:Mwsn}
${BUILD}/bin/kgt: ${BUILD}/lib/${part}.o
.endfor

