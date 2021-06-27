.MAKEFLAGS: -r -m share/mk

# targets
all::  mkdir .WAIT dep .WAIT prog
dep::
gen::
test::
install:: all
clean::

# things to override
CC       ?= gcc
BUILD    ?= build
PREFIX   ?= /usr/local
WASMTIME ?= wasmtime

# layout
SUBDIR += src/bnf
SUBDIR += src/blab
SUBDIR += src/ebnfhtml5
SUBDIR += src/dot
SUBDIR += src/abnf
SUBDIR += src/iso-ebnf
SUBDIR += src/rbnf
SUBDIR += src/sid
SUBDIR += src/wsn
SUBDIR += src/rrd
SUBDIR += src/rrdump
SUBDIR += src/rrtdump
SUBDIR += src/rrparcon
SUBDIR += src/rrll
SUBDIR += src/rrta
SUBDIR += src/rrtext
SUBDIR += src/rrdot
SUBDIR += src/svg
SUBDIR += src/html5
SUBDIR += src/json
SUBDIR += src

.include <subdir.mk>
.include <sid.mk>
.include <lx.mk>
.include <obj.mk>
.include <dep.mk>
.include <ar.mk>
.include <so.mk>
.include <part.mk>
.include <prog.mk>
.include <mkdir.mk>
.include <install.mk>
.include <clean.mk>

test::
.if ${CC:T:Memcc}
	echo 'hello = world.' | ${WASMTIME} ${BUILD}/bin/kgt.wasm -- -l wsn -e rrutf8
.endif

