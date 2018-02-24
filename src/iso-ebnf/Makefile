.include "../../share/mk/top.mk"

SRC += src/iso-ebnf/lexer.c
SRC += src/iso-ebnf/parser.c
SRC += src/iso-ebnf/output.c

LEXER  += src/iso-ebnf/lexer.lx
PARSER += src/iso-ebnf/parser.sid

PART += iso-ebnf

.for src in ${LEXER:Msrc/iso-ebnf/lexer.lx}
LX_CFLAGS.${src} += -b dyn -e lx_iso_ebnf_ -t TOK_
.endfor

.for parser in ${PARSER:Msrc/iso-ebnf/parser.sid}
ACT.${parser} = src/parser.act
.endfor

.for src in ${SRC:Msrc/iso-ebnf/lexer.c}
CFLAGS.${src} += -D LX_HEADER='"lexer.h"'
DFLAGS.${src} += -D LX_HEADER='"lexer.h"'
.endfor

.for src in ${SRC:Msrc/iso-ebnf/parser.c}
CFLAGS.${src} += -D FORM='iso_ebnf'
DFLAGS.${src} += -D FORM='iso_ebnf'
.endfor

.for src in ${SRC:Msrc/iso-ebnf/*.c}
${BUILD}/lib/iso-ebnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/iso-ebnf.opic: ${BUILD}/${src:R}.opic
.endfor

