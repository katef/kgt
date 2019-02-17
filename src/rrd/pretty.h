/*
 * Copyright 2014-2017 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_RRD_PRETTY_H
#define KGT_RRD_PRETTY_H

struct node;

void rrd_pretty_ci(int *changed, struct node **);
void rrd_pretty_affixes(int *changed, struct node **);
void rrd_pretty_bottom(int *changed, struct node **);
void rrd_pretty_redundant(int *changed, struct node **);
void rrd_pretty_skippable(int *changed, struct node **);
void rrd_pretty_collapse(int *changed, struct node **);
void rrd_pretty_nested(int *changed, struct node **);
void rrd_pretty_roll(int *changed, struct node **);

void rrd_pretty(struct node **rrd);

#endif
