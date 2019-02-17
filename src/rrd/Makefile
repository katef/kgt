.include "../../share/mk/top.mk"

SRC += src/rrd/node.c
SRC += src/rrd/transform.c
SRC += src/rrd/list.c
SRC += src/rrd/tnode.c

SRC += src/rrd/rewrite_ci.c

SRC += src/rrd/pretty_ci.c
SRC += src/rrd/pretty_collapse.c
SRC += src/rrd/pretty_affix.c
SRC += src/rrd/pretty_bottom.c
SRC += src/rrd/pretty_nested.c
SRC += src/rrd/pretty_redundant.c
SRC += src/rrd/pretty_skippable.c
SRC += src/rrd/pretty_roll.c
SRC += src/rrd/pretty.c

PART += rrd

.for src in ${SRC:Msrc/rrd/*.c}
${BUILD}/lib/rrd.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rrd.opic: ${BUILD}/${src:R}.opic
.endfor

