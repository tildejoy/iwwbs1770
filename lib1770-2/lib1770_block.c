/*
 * lib1770_block.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 * Copyright (C) 2019 Tilde Joy <tilde@ultros.pro>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#include "lib1770.h"

lib1770_block_t *
lib1770_block_new(double samplerate, double ms, int partition)
{
	lib1770_block_t *block;

	LIB1770_GOTO(0==partition, "missing partition", partition);
 	block = LIB1770_CALLOC(1, (sizeof *block) + \
	    LIB1770_AGG_BLOCK_SIZE(partition));
	LIB1770_GOTO(NULL == block, "allocating bs.1770 block", block);
	block->next = NULL;
	block->stats = NULL;

	block->gate = LIB1770_SILENCE_GATE;
	block->partition = partition;
	block->length = 0.001 * ms;

	block->samplerate = samplerate;
	/* 400/partition ms. */
 	block->overlap_size = round(block->length * samplerate / partition);
	/* 400 ms. */
	block->block_size = partition * block->overlap_size;
	block->scale = 1.0 / (double)block->block_size;

	block->ring.size = partition;
	block->ring.offs = 0;
	block->ring.wmsq[block->ring.offs] = 0.0;
	block->ring.count = 0;
	block->ring.used = 1;

	return block;
	/* LIB1770_FREE(block); */
block:
partition:
	return NULL;
}

void 
lib1770_block_close(lib1770_block_t *block)
{
	LIB1770_FREE(block);
}

void 
lib1770_block_add_stats(lib1770_block_t *block, lib1770_stats_t *stats)
{
	stats->next = block->stats;
	block->stats = stats;
}

void
lib1770_block_add_sqs(lib1770_block_t *block, double wssqs)
{
	double *wmsq = block->ring.wmsq;
	lib1770_stats_t *stats;

	if (1.0e-15 <= wssqs) {
		double *wp = wmsq;
		double *mp = wp + block->ring.used;

		wssqs *= block->scale;

		while (wp < mp)
			(*wp++) += wssqs;
	}

	if (++block->ring.count == block->overlap_size) {
		size_t next_offs = block->ring.offs + 1;

		if (next_offs == block->ring.size)
			next_offs=0;

		if (block->ring.used == block->ring.size) {
			double prev_wmsq=wmsq[next_offs];

			if (block->gate < prev_wmsq) {
				for (stats = block->stats; NULL != stats; \
				    stats = stats->next)
#if defined (LIB1770_STATS_VMT)
					stats->vmt->add_sqs(stats,prev_wmsq);
#else
					lib1770_stats_add_sqs(stats,prev_wmsq);
#endif
			}
		}

		block->ring.wmsq[next_offs] = 0.0;
		block->ring.count = 0;
		block->ring.offs = next_offs;

		if (block->ring.used < block->ring.size)
			++block->ring.used;
	}
}
