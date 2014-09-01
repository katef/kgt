.include "../../share/mk/top.mk"

SRC += src/trd/output.c

PART += trd

.for src in ${SRC:Msrc/trd/*.c}
${BUILD}/lib/trd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/trd.opic: ${BUILD}/${src:R}.opic
.endfor

