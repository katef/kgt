/* $Id$ */

#ifndef KGT_RRD_PRETTY_H
#define KGT_RRD_PRETTY_H

#include "rrd.h"

void rrd_pretty_prefixes(struct node **);
void rrd_pretty_suffixes(struct node **);
void rrd_pretty_bottom(struct node **);
void rrd_pretty_redundant(struct node **);
void rrd_pretty_collapse(struct node **);

#endif
