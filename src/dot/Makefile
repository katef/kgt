.include "../../share/mk/top.mk"

SRC += src/dot/output.c

PART += dot

${BUILD}/src/dot/lexer.o: src/dot/lexer.h

.for src in ${SRC:Msrc/dot/*.c}
${BUILD}/lib/dot.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/dot.opic: ${BUILD}/${src:R}.opic
.endfor

