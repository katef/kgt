.include "../../share/mk/top.mk"

SRC += src/blab/output.c

PART += blab

.for src in ${SRC:Msrc/blab/*.c}
${BUILD}/lib/blab.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/blab.opic: ${BUILD}/${src:R}.opic
.endfor

