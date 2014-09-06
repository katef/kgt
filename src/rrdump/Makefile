.include "../../share/mk/top.mk"

SRC += src/rrdump/output.c

PART += rrdump

.for src in ${SRC:Msrc/rrdump/*.c}
${BUILD}/lib/rrdump.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrdump.opic: ${BUILD}/${src:R}.opic
.endfor

