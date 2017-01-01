/* $Id: pretty_suffix.c 185 2016-12-23 21:32:07Z kate $ */

/*
 * Railroad diagram beautification
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../xalloc.h"

#include "rrd.h"
#include "pretty.h"

void
rrd_pretty(struct node **rrd)
{
	int changed;
	int limit;

	limit = 20;

	do {
		changed = 0;

		rrd_pretty_collapse(&changed, rrd);
		rrd_pretty_prefixes(&changed, rrd);
		rrd_pretty_suffixes(&changed, rrd);
		rrd_pretty_roll(&changed, rrd);
		rrd_pretty_redundant(&changed, rrd);
		rrd_pretty_bottom(&changed, rrd);
	} while (changed && !limit--);
}

