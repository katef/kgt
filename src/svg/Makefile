.include "../../share/mk/top.mk"

SRC += src/svg/output.c

PART += svg

.for src in ${SRC:Msrc/svg/*.c}
${BUILD}/lib/svg.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/svg.opic: ${BUILD}/${src:R}.opic
.endfor

