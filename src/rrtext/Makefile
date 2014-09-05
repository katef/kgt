.include "../../share/mk/top.mk"

SRC += src/rrtext/output.c
SRC += src/rrtext/print_text.c

PART += rrtext

.for src in ${SRC:Msrc/rrtext/*.c}
${BUILD}/lib/rrtext.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrtext.opic: ${BUILD}/${src:R}.opic
.endfor

