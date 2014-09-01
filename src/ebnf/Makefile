.include "../../share/mk/top.mk"

SRC += src/ebnf/lexer.c
SRC += src/ebnf/parser.c
SRC += src/ebnf/output.c

# LEXIFLAGS = -t tok_ -p ebnf_

PART += ebnf

.for src in ${SRC:Msrc/ebnf/benf_lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
.endfor

${BUILD}/src/ebnf/lexer.o: src/ebnf/lexer.h

.for src in ${SRC:Msrc/ebnf/*.c}
${BUILD}/lib/ebnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/ebnf.opic: ${BUILD}/${src:R}.opic
.endfor

