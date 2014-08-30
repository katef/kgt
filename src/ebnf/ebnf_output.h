/* $Id$ */

/*
 * Extended Backus-Naur Form Output
 * As defined by ISO/IEC 14977:1996(E)
 */

#ifndef KGT_EBNF_OUTPUT_H
#define KGT_EBNF_OUTPUT_H

#include "../ast.h"

void ebnf_output(struct ast_production *grammar);

#endif

