.include "../../share/mk/top.mk"

SRC += src/rrd/rrd_transform.c
SRC += src/rrd/rrd_beautify.c
SRC += src/rrd/rrd_output.c

PART += rrd

.for src in ${SRC:Msrc/rrd/*.c}
${BUILD}/lib/rrd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrd.opic: ${BUILD}/${src:R}.opic
.endfor

