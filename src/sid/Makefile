.include "../../share/mk/top.mk"

SRC += src/sid/output.c

PART += sid

.for src in ${SRC:Msrc/sid/*.c}
${BUILD}/lib/sid.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/sid.opic: ${BUILD}/${src:R}.opic
.endfor

