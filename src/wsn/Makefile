.include "../../share/mk/top.mk"

SRC += src/wsn/lexer.c
SRC += src/wsn/parser.c
SRC += src/wsn/output.c

LEXER  += src/wsn/lexer.lx
PARSER += src/wsn/parser.sid

PART += wsn

.for src in ${LEXER:Msrc/wsn/lexer.lx}
LX_CFLAGS.${src} += -b dyn -e lx_wsn_ -t TOK_
.endfor

.for parser in ${PARSER:Msrc/wsn/parser.sid}
ACT.${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/wsn/lexer.c}
CFLAGS.${src} += -D LX_HEADER='"lexer.h"'
DFLAGS.${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/wsn/parser.c}
CFLAGS.${src} += -D FORM='wsn'
DFLAGS.${src} += -D FORM='wsn'
.endfor

.for src in ${SRC:Msrc/wsn/*.c}
${BUILD}/lib/wsn.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/wsn.opic: ${BUILD}/${src:R}.opic
.endfor

