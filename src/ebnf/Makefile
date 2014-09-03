.include "../../share/mk/top.mk"

SRC += src/ebnf/lexer.c
SRC += src/ebnf/parser.c
SRC += src/ebnf/output.c

LEXER  += src/ebnf/lexer.lx
PARSER += src/ebnf/parser.sid

PART += ebnf

.for src in ${LEXER:Msrc/ebnf/lexer.lx}
LX_CFLAGS_${src} += -p lx_ebnf_
.endfor

.for parser in ${PARSER:Msrc/ebnf/parser.sid}
ACT_${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/ebnf/lexer.c}
CFLAGS_${src} += -D LX_HEADER='"lexer.h"'
DFLAGS_${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/ebnf/parser.c}
CFLAGS_${src} += -D FORM='ebnf'
DFLAGS_${src} += -D FORM='ebnf'
.endfor

.for src in ${SRC:Msrc/ebnf/*.c}
${BUILD}/lib/ebnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/ebnf.opic: ${BUILD}/${src:R}.opic
.endfor

