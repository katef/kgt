.include "../../share/mk/top.mk"

SRC += src/rbnf/lexer.c
SRC += src/rbnf/parser.c
SRC += src/rbnf/output.c

LEXER  += src/rbnf/lexer.lx
PARSER += src/rbnf/parser.sid

PART += rbnf

.for src in ${LEXER:Msrc/rbnf/lexer.lx}
LX_CFLAGS.${src} += -b dyn -e lx_rbnf_ -t TOK_
.endfor

.for parser in ${PARSER:Msrc/rbnf/parser.sid}
ACT.${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/rbnf/lexer.c}
CFLAGS.${src} += -D LX_HEADER='"lexer.h"'
DFLAGS.${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/rbnf/parser.c}
CFLAGS.${src} += -D FORM='rbnf'
DFLAGS.${src} += -D FORM='rbnf'
.endfor

.for src in ${SRC:Msrc/rbnf/*.c}
${BUILD}/lib/rbnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/rbnf.opic: ${BUILD}/${src:R}.opic
.endfor

