.include "../../share/mk/top.mk"

SRC += src/rrparcon/output.c

PART += rrparcon

.for src in ${SRC:Msrc/rrparcon/*.c}
${BUILD}/lib/rrparcon.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrparcon.opic: ${BUILD}/${src:R}.opic
.endfor

