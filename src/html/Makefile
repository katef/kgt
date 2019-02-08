.include "../../share/mk/top.mk"

SRC += src/html/output.c

PART += html

.for src in ${SRC:Msrc/html/*.c}
${BUILD}/lib/html.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/html.opic: ${BUILD}/${src:R}.opic
.endfor

