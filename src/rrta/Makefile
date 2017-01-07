.include "../../share/mk/top.mk"

SRC += src/rrta/output.c

PART += rrta

.for src in ${SRC:Msrc/rrta/*.c}
${BUILD}/lib/rrta.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrta.opic: ${BUILD}/${src:R}.opic
.endfor

