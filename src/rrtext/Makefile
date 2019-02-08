.include "../../share/mk/top.mk"

SRC += src/rrtext/output.c

PART += rrtext

.for src in ${SRC:Msrc/rrtext/*.c}
${BUILD}/lib/rrtext.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrtext.opic: ${BUILD}/${src:R}.opic
.endfor

