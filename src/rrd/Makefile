.include "../../share/mk/top.mk"

SRC += src/rrd/transform.c
SRC += src/rrd/beautify.c
SRC += src/rrd/output.c
SRC += src/rrd/render.c

PART += rrd

.for src in ${SRC:Msrc/rrd/*.c}
${BUILD}/lib/rrd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrd.opic: ${BUILD}/${src:R}.opic
.endfor

