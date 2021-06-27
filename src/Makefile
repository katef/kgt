.include "../share/mk/top.mk"

SRC += src/ast.c
SRC += src/ast_binary.c
SRC += src/bitmap.c
SRC += src/main.c
SRC += src/parsing_error.c
SRC += src/rewrite_ci.c
SRC += src/rewrite_invisible.c
SRC += src/txt.c
SRC += src/xalloc.c

PROG += kgt

# XXX: ${SRC:Msrc/*.c} matches src/xyz/*.c which is not what we want
#.for src in ${SRC:Msrc/*.c}
#${BUILD}/bin/kgt: ${BUILD}/${src:R}.o
#.endfor

LFLAGS.kgt += -lm

${BUILD}/bin/kgt: ${BUILD}/src/ast.o
${BUILD}/bin/kgt: ${BUILD}/src/ast_binary.o
${BUILD}/bin/kgt: ${BUILD}/src/bitmap.o
${BUILD}/bin/kgt: ${BUILD}/src/main.o
${BUILD}/bin/kgt: ${BUILD}/src/parsing_error.o
${BUILD}/bin/kgt: ${BUILD}/src/rewrite_ci.o
${BUILD}/bin/kgt: ${BUILD}/src/rewrite_invisible.o
${BUILD}/bin/kgt: ${BUILD}/src/txt.o
${BUILD}/bin/kgt: ${BUILD}/src/xalloc.o

.for part in ${PART:Mbnf} ${PART:Mblab} ${PART:Mabnf} ${PART:Miso-ebnf} ${PART:Mrbnf} \
	${PART:Msid} ${PART:Mdot} ${PART:Mwsn} \
	${PART:Mebnfhtml5} \
	${PART:Mrrd} ${PART:Mrrdot} ${PART:Mrrdump} ${PART:Mrrtdump} \
	${PART:Mrrparcon} ${PART:Mrrll} ${PART:Mrrta} ${PART:Mrrtext} \
	${PART:Msvg} ${PART:Mhtml5} ${PART:Mjson}
${BUILD}/bin/kgt: ${BUILD}/lib/${part}.o
.endfor

