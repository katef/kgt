.include "../../share/mk/top.mk"

SRC += src/rrll/output.c

PART += rrll

.for src in ${SRC:Msrc/rrll/*.c}
${BUILD}/lib/rrll.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrll.opic: ${BUILD}/${src:R}.opic
.endfor

