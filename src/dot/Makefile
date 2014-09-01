.include "../../share/mk/top.mk"

SRC += src/dot/dot_output.c

# LEXIFLAGS = -t tok_ -p dot_

PART += dot

${BUILD}/lib/dot_lexer.o: dot_lexer.h

.for src in ${SRC:Msrc/dot/*.c}
${BUILD}/lib/dot.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/dot.opic: ${BUILD}/${src:R}.opic
.endfor

