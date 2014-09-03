/* $Id$ */

#ifndef KGT_WSN_INPUT_H
#define KGT_WSN_INPUT_H

struct ast_production *
wsn_input(int (*f)(void *opaque), void *opaque);

#endif

