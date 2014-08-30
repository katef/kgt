/* $Id$ */

#ifndef KGT_IO_H
#define KGT_IO_H

#include <stdio.h>

#include "tokens.h"

/*
 * The input file stream.
 */
extern FILE *io_fin;
extern unsigned int io_line;

/*
 * The token buffer, containing a null-terminated string of the literal spelling
 * of tokens we wish to use; these are identifiers, literal strings and so on.
 */
extern char io_buffer[];

/*
 * Push a character to the token buffer.
 */
void io_push(int c);

/*
 * Flush the token buffer.
 */
void io_flush(void);

/*
 * Produce a literal token, or an empty token if the token buffer has no content.
 */
enum tok io_literal(void);

#endif

