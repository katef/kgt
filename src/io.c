/* $Id$ */

/*
 * Input/Output.
 *
 * This is centralised between all lexers, as we only use one at a time.
 */

#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "main.h"
#include "tokens.h"
#include "xalloc.h"

FILE *io_fin;
unsigned int io_line;

char io_buffer[1024];

size_t bufferindex;

void
io_push(int c)
{
	if (bufferindex + 1 == sizeof io_buffer) {
		xerror("literal string value too long");
	}

	io_buffer[bufferindex++] = c;
	io_buffer[bufferindex]   = '\0';
}

void
io_flush(void)
{
	bufferindex = 0;
	io_buffer[0] = '\0';
}

enum tok
io_literal(void)
{
	return bufferindex == 0 ? tok_empty : tok_literal;
}

