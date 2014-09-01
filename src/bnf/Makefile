.include "../../share/mk/top.mk"

SRC += src/bnf/lexer.c
SRC += src/bnf/parser.c
SRC += src/bnf/output.c

# LEXIFLAGS = -t tok_ -p bnf_

PART += bnf

.for src in ${SRC:Msrc/bnf/lexer.c}
CFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
DFLAGS_${src} += -D LEXI_HEADER='"lexer.h"'
.endfor

${BUILD}/src/bnf/lexer.o: src/bnf/lexer.h

.for src in ${SRC:Msrc/bnf/*.c}
${BUILD}/lib/bnf.o:    ${BUILD}/${src:R}.o
${BUILD}/lib/bnf.opic: ${BUILD}/${src:R}.opic
.endfor


#lexer.c: lexer.lxi ../lexer.lct parser.h
#	$(LEXI) $(LEXIFLAGS) -l c lexer.lxi ../lexer.lct lexer.c
#	sed -i 's/LEXI_HEADER/"lexer.h"/' ${.TARGET}
#
#lexer.h: lexer.lxi ../lexer.lct parser.h
#	$(LEXI) $(LEXIFLAGS) -l h lexer.lxi ../lexer.lct lexer.h
#
#parser.c parser.h: parser.sid ../parser.act
#	$(SID) $(SIDFLAGS) parser.sid ../parser.act parser.c parser.h

