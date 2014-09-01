.include "../../share/mk/top.mk"

SRC += src/trd/trd_output.c

# LEXIFLAGS = -t tok_ -p trd_

PART += trd

.for src in ${SRC:Msrc/trd/*.c}
${BUILD}/lib/trd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/trd.opic: ${BUILD}/${src:R}.opic
.endfor

