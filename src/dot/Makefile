.include "../../share/mk/top.mk"

SRC += src/dot/dot_output.c

PART += dot

${BUILD}/src/dot/dot_lexer.o: src/dot/dot_lexer.h

.for src in ${SRC:Msrc/dot/*.c}
${BUILD}/lib/dot.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/dot.opic: ${BUILD}/${src:R}.opic
.endfor

