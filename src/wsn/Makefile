.include "../../share/mk/top.mk"

SRC += src/wsn/lexer.c
SRC += src/wsn/parser.c
SRC += src/wsn/output.c

# LEXIFLAGS = -t tok_ -p wsn_

PART += wsn

.for src in ${SRC:Msrc/wsn/lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
.endfor

${BUILD}/src/wsn/lexer.o: src/wsn/lexer.h

.for src in ${SRC:Msrc/wsn/*.c}
${BUILD}/lib/wsn.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/wsn.opic: ${BUILD}/${src:R}.opic
.endfor

