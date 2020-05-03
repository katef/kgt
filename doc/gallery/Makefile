
BASE ?= ../../
BIN ?= ${BASE}/build/bin
EXAMPLES ?= ${BASE}/examples
KGT ?= ${BIN}/kgt

all: \
	c89-ebnf.html c89-rrd.html c89-ascii.txt c89-utf8.txt \
	c99-ebnf.html c99-rrd.html c99-ascii.txt c99-utf8.txt

c89-ebnf.html: ${EXAMPLES}/c_syntax.wsn
	${KGT} -l wsn -e ebnfhtml5 < $< > $@

c89-rrd.html: ${EXAMPLES}/c_syntax.wsn
	${KGT} -l wsn -e html5 < $< > $@

c89-ascii.txt: ${EXAMPLES}/c_syntax.wsn
	${KGT} -l wsn -e rrtext < $< > $@

c89-utf8.txt: ${EXAMPLES}/c_syntax.wsn
	${KGT} -l wsn -e rrutf8 < $< > $@

c99-ebnf.html: ${EXAMPLES}/c99-grammar.iso-ebnf
	${KGT} -l iso-ebnf -e ebnfhtml5 < $< > $@

c99-rrd.html: ${EXAMPLES}/c99-grammar.iso-ebnf
	${KGT} -l iso-ebnf -e html5 < $< > $@

c99-ascii.txt: ${EXAMPLES}/c99-grammar.iso-ebnf
	${KGT} -l iso-ebnf -e rrtext < $< > $@

c99-utf8.txt: ${EXAMPLES}/c99-grammar.iso-ebnf
	${KGT} -l iso-ebnf -e rrutf8 < $< > $@

