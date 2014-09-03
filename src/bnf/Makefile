.include "../../share/mk/top.mk"

SRC += src/bnf/lexer.c
SRC += src/bnf/parser.c
SRC += src/bnf/output.c

LEXER  += src/bnf/lexer.lx
PARSER += src/bnf/parser.sid

PART += bnf

.for src in ${LEXER:Msrc/bnf/lexer.lx}
LX_CFLAGS_${src} += -p lx_bnf_
.endfor

.for parser in ${PARSER:Msrc/bnf/parser.sid}
ACT_${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/bnf/lexer.c}
CFLAGS_${src} += -D LX_HEADER='"lexer.h"'
DFLAGS_${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/bnf/parser.c}
CFLAGS_${src} += -D FORM='bnf'
DFLAGS_${src} += -D FORM='bnf'
.endfor

.for src in ${SRC:Msrc/bnf/*.c}
${BUILD}/lib/bnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/bnf.opic: ${BUILD}/${src:R}.opic
.endfor

