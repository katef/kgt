
## KGT: Kate's Grammar Tool

Do you want to convert various kinds of BNF to other kinds of BNF? No?  
Well imagine if you did! This would be the tool for you.

 Input:  Various BNF-like syntaxes  
 Output: Various BNF-like syntaxes, AST dumps, and Railroad Syntax Diagrams

Getting started:

See the [/examples](examples/) directory for grammars in various
BNF dialects. These have been collated from various sources and
are of various quality. BNF dialects tend to be poorly specified,
and these examples are an attempt to keep a corpus of known-good
examples for each dialect. kgt can't parse them all yet.

kgt reads from _stdin_ in dialect `-l ...` and writes to _stdout_
in format `-e ...`:

    ; kgt -l bnf -e rrtext < examples/expr.bnf
    expr:
        ||--v-- term -- "+" -- expr -->--||
            |                         |
            `--------- term ----------'
    
    term:
        ||--v-- factor -- "*" -- term -->--||
            |                           |
            `--------- factor ----------'
    
    factor:
        ||--v-- "(" -- expr -- ")" -->--||
            |                        |
            `-------- const ---------'
    
    const:
        ||-- integer --||

Clone with submodules (contains required .mk files):

    ; git clone --recursive https://github.com/katef/kgt.git

To build and install:

    ; pmake -r install

You can override a few things:

    ; CC=clang PREFIX=$HOME pmake -r install

Building depends on:

 * Any BSD make. This includes OpenBSD, FreeBSD and NetBSD make(1)
   and sjg's portable bmake (also packaged as pmake).

 * A C compiler. Any should do, but GCC and clang are best supported.

 * ar, ld, and a bunch of other stuff you probably already have.

Ideas, comments or bugs: kate@elide.org

