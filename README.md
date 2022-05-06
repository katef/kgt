
## KGT: Kate's Grammar Tool

Do you want to convert various kinds of BNF to other kinds of BNF? No?  
Well imagine if you did! This would be the tool for you.

 Input:  Various BNF-like syntaxes  
 Output: Various BNF-like syntaxes, AST dumps, and Railroad Syntax Diagrams

Compilation phases:

![phases.svg](doc/tutorial/phases.svg)

- Bold indicates the input BNF dialects with the most features
- ✨ indicates presentational formats (these are the good ones!)
- 🧪 indicates debugging formats.  
  You don't want these unless you're looking at kgt's internals.
- Other formats provide various subsets of features

Gallery:

- C89 grammar [WSN source](/examples/c_syntax.wsn)  
  → Presentational BNF:
  [EBNF](https://katef.github.io/kgt/doc/gallery/c89-ebnf.html)  
  → Railroad diagrams:
  [SVG](https://katef.github.io/kgt/doc/gallery/c89-rrd.html),
  [ASCII](https://katef.github.io/kgt/doc/gallery/c89-ascii.txt),
  [UTF8](https://katef.github.io/kgt/doc/gallery/c89-utf8.txt)   ✨ Look at these ones! ✨
- C99 grammar [EBNF source](/examples/c99-grammar.iso-ebnf)  
  → Presentational BNF:
  [EBNF](https://katef.github.io/kgt/doc/gallery/c99-ebnf.html)  
  → Railroad diagrams:
  [SVG](https://katef.github.io/kgt/doc/gallery/c99-rrd.html),
  [ASCII](https://katef.github.io/kgt/doc/gallery/c99-ascii.txt),
  [UTF8](https://katef.github.io/kgt/doc/gallery/c99-utf8.txt)

Getting started:

See the [/examples](examples/) directory for grammars in various
BNF dialects. These have been collated from various sources and
are of various quality. BNF dialects tend to be poorly specified,
and these examples are an attempt to keep a corpus of known-good
examples for each dialect. kgt can't parse them all yet.

kgt reads from _stdin_ in dialect `-l ...` and writes to _stdout_
in format `-e ...`:

    ; kgt -l bnf -e rrutf8 < examples/expr.bnf
    expr:
        │├──╮── term ── "+" ── expr ──╭──┤│
            │                         │
            ╰───────── term ──────────╯
    
    term:
        │├──╮── factor ── "*" ── term ──╭──┤│
            │                           │
            ╰───────── factor ──────────╯
    
    factor:
        │├──╮── "(" ── expr ── ")" ──╭──┤│
            │                        │
            ╰──────── const ─────────╯
    
    const:
        │├── integer ──┤│

and the same grammar output as SVG instead:

    ; kgt -l bnf -e svg < examples/expr.bnf > /tmp/expr.svg

![expr.svg](examples/expr.svg)

### Building from source

Clone with submodules (contains required .mk files):

    ; git clone --recursive https://github.com/katef/kgt.git

To build and install:

    ; bmake -r install

You can override a few things:

    ; CC=clang bmake -r
    ; PREFIX=$HOME bmake -r install

You need bmake for building. In order of preference:

 1. If you use some kind of BSD (NetBSD, OpenBSD, FreeBSD, ...) this is make(1).
    They all differ slightly. Any of them should work.
 2. If you use Linux or MacOS and you have a package named bmake, use that.
 3. If you use Linux and you have a package named pmake, use that.
    It's the same thing.
    Some package managers have bmake packaged under the name pmake.
    I don't know why they name it pmake.
 4. Otherwise if you use MacOS and you only have a package named bsdmake, use that.
    It's Apple's own fork of bmake.
    It should also work but it's harder for me to test.
 5. If none of these are options for you, you can build bmake from source.
    You don't need mk.tar.gz, just bmake.tar.gz. This will always work.
    https://www.crufty.net/help/sjg/bmake.html

When you see "bmake" in the build instructions above, it means any of these.

Building depends on:

 * Any BSD make.
 * A C compiler. Any should do, but GCC and clang are best supported.
 * ar, ld, and a bunch of other stuff you probably already have.

Ideas, comments or bugs: kate@elide.org

