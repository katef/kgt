.include "../../share/mk/top.mk"

SRC += src/abnf/lexer.c
SRC += src/abnf/parser.c

LEXER  += src/abnf/lexer.lx
PARSER += src/abnf/parser.sid

PART += abnf

.for src in ${LEXER:Msrc/abnf/lexer.lx}
LX_CFLAGS_${src} += -p lx_abnf_
.endfor

.for parser in ${PARSER:Msrc/abnf/parser.sid}
ACT_${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/abnf/lexer.c}
CFLAGS_${src} += -D LX_HEADER='"lexer.h"'
DFLAGS_${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/abnf/parser.c}
CFLAGS_${src} += -D FORM='abnf'
DFLAGS_${src} += -D FORM='abnf'
.endfor

.for src in ${SRC:Msrc/abnf/*.c}
${BUILD}/lib/abnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/abnf.opic: ${BUILD}/${src:R}.opic
.endfor

