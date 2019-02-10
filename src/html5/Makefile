.include "../../share/mk/top.mk"

SRC += src/html5/output.c

PART += html5

.for src in ${SRC:Msrc/html5/*.c}
${BUILD}/lib/html5.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/html5.opic: ${BUILD}/${src:R}.opic
.endfor

