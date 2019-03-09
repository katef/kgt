.include "../../share/mk/top.mk"

SRC += src/abnf/lexer.c
SRC += src/abnf/parser.c
SRC += src/abnf/output.c

LEXER  += src/abnf/lexer.lx
PARSER += src/abnf/parser.sid

PART += abnf

.for src in ${LEXER:Msrc/abnf/lexer.lx}
LX_CFLAGS.${src} += -b dyn -e lx_abnf_ -t TOK_
.endfor

.for parser in ${PARSER:Msrc/abnf/parser.sid}
ACT.${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/abnf/lexer.c}
CFLAGS.${src} += -D LX_HEADER='"lexer.h"'
DFLAGS.${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/abnf/parser.c}
CFLAGS.${src} += -D FORM='abnf'
DFLAGS.${src} += -D FORM='abnf'
.endfor

.for src in ${SRC:Msrc/abnf/*.c}
${BUILD}/lib/abnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/abnf.opic: ${BUILD}/${src:R}.opic
.endfor

