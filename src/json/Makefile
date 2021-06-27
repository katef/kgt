.include "../../share/mk/top.mk"

SRC += src/json/output.c

PART += json

.for src in ${SRC:Msrc/json/*.c}
${BUILD}/lib/json.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/json.opic: ${BUILD}/${src:R}.opic
.endfor
