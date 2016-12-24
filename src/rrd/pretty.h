/* $Id$ */

#ifndef KGT_RRD_PRETTY_H
#define KGT_RRD_PRETTY_H

#include "rrd.h"

void rrd_pretty_prefixes(int *changed, struct node **);
void rrd_pretty_suffixes(int *changed, struct node **);
void rrd_pretty_bottom(int *changed, struct node **);
void rrd_pretty_redundant(int *changed, struct node **);
void rrd_pretty_collapse(int *changed, struct node **);

void rrd_pretty(struct node **rrd);

#endif
