.include "../../share/mk/top.mk"

SRC += src/rrtdump/output.c

PART += rrtdump

.for src in ${SRC:Msrc/rrtdump/*.c}
${BUILD}/lib/rrtdump.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrtdump.opic: ${BUILD}/${src:R}.opic
.endfor

