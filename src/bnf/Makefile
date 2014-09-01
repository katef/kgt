.include "../../share/mk/top.mk"

SRC += src/bnf/bnf_lexer.c
SRC += src/bnf/bnf_parser.c
SRC += src/bnf/bnf_output.c

# LEXIFLAGS = -t tok_ -p bnf_

PART += bnf

.for src in ${SRC:Msrc/bnf/bnf_lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"bnf_lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"bnf_lexer.h"'
.endfor

${BUILD}/lib/bnf_lexer.o: bnf_lexer.h

.for src in ${SRC:Msrc/bnf/*.c}
${BUILD}/lib/bnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/bnf.opic: ${BUILD}/${src:R}.opic
.endfor


#bnf_lexer.c: bnf_lexer.lxi ../lexer.lct bnf_parser.h
#	$(LEXI) $(LEXIFLAGS) -l c bnf_lexer.lxi ../lexer.lct bnf_lexer.c
#	sed -i 's/LEXI_HEADER/"bnf_lexer.h"/' ${.TARGET}
#
#bnf_lexer.h: bnf_lexer.lxi ../lexer.lct bnf_parser.h
#	$(LEXI) $(LEXIFLAGS) -l h bnf_lexer.lxi ../lexer.lct bnf_lexer.h
#
#bnf_parser.c bnf_parser.h: bnf_parser.sid ../parser.act
#	$(SID) $(SIDFLAGS) bnf_parser.sid ../parser.act bnf_parser.c bnf_parser.h

