.include "../../share/mk/top.mk"

SRC += src/bnf/lexer.c
SRC += src/bnf/parser.c
SRC += src/bnf/output.c

LEXER  += src/bnf/lexer.lx
PARSER += src/bnf/parser.sid

PART += bnf

.for src in ${LEXER:Msrc/bnf/lexer.lx}
LX_CFLAGS.${src} += -b dyn -e lx_bnf_ -t TOK_
.endfor

.for parser in ${PARSER:Msrc/bnf/parser.sid}
ACT.${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/bnf/lexer.c}
CFLAGS.${src} += -D LX_HEADER='"lexer.h"'
DFLAGS.${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/bnf/parser.c}
CFLAGS.${src} += -D FORM='bnf'
DFLAGS.${src} += -D FORM='bnf'
.endfor

.for src in ${SRC:Msrc/bnf/*.c}
${BUILD}/lib/bnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/bnf.opic: ${BUILD}/${src:R}.opic
.endfor

