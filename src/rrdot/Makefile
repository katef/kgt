.include "../../share/mk/top.mk"

SRC += src/rrdot/output.c
SRC += src/rrdot/print_dot.c

PART += rrdot

.for src in ${SRC:Msrc/rrdot/*.c}
${BUILD}/lib/rrdot.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrdot.opic: ${BUILD}/${src:R}.opic
.endfor

