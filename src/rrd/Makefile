.include "../../share/mk/top.mk"

SRC += src/rrd/blist.c
SRC += src/rrd/transform.c
SRC += src/rrd/output.c
SRC += src/rrd/node.c

SRC += src/rrd/print_dump.c
SRC += src/rrd/print_text.c

SRC += src/rrd/pretty_suffix.c
SRC += src/rrd/pretty_bottom.c
SRC += src/rrd/pretty_redundant.c

PART += rrd

.for src in ${SRC:Msrc/rrd/*.c}
${BUILD}/lib/rrd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrd.opic: ${BUILD}/${src:R}.opic
.endfor

