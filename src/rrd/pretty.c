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
	rrd_pretty_prefixes(&rrd);
	rrd_pretty_suffixes(&rrd);
	rrd_pretty_redundant(&rrd);
	rrd_pretty_bottom(&rrd);
	rrd_pretty_collapse(&rrd);
}

