.include "../../share/mk/top.mk"

SRC += src/wsn/wsn_lexer.c
SRC += src/wsn/wsn_parser.c
SRC += src/wsn/wsn_output.c

# LEXIFLAGS = -t tok_ -p wsn_

PART += wsn

.for src in ${SRC:Msrc/wsn/wsn_lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"wsn_lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"wsn_lexer.h"'
.endfor

${BUILD}/src/wsn/wsn_lexer.o: src/wsn/wsn_lexer.h

.for src in ${SRC:Msrc/wsn/*.c}
${BUILD}/lib/wsn.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/wsn.opic: ${BUILD}/${src:R}.opic
.endfor

