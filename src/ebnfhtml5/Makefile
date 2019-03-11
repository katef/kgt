.include "../../share/mk/top.mk"

SRC += src/ebnfhtml5/output.c

PART += ebnfhtml5

.for src in ${SRC:Msrc/ebnfhtml5/*.c}
${BUILD}/lib/ebnfhtml5.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/ebnfhtml5.opic: ${BUILD}/${src:R}.opic
.endfor

