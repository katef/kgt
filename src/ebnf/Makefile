.include "../../share/mk/top.mk"

SRC += src/ebnf/ebnf_lexer.c
SRC += src/ebnf/ebnf_parser.c
SRC += src/ebnf/ebnf_output.c

# LEXIFLAGS = -t tok_ -p ebnf_

PART += ebnf

.for src in ${SRC:Msrc/ebnf/benf_lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"ebnf_lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"ebnf_lexer.h"'
.endfor

${BUILD}/src/ebnf/ebnf_lexer.o: src/ebnf/ebnf_lexer.h

.for src in ${SRC:Msrc/ebnf/*.c}
${BUILD}/lib/ebnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/ebnf.opic: ${BUILD}/${src:R}.opic
.endfor

