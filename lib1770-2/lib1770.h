/*
 * lib1770.h
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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
#ifndef __LIB1770_H__ // {
#define __LIB1770_H__
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pbutil.h>
#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined (_MSC_VER) && ! defined (__func__) // {
  #define __func__ __FUNCTION__
#endif // }

#define LIB1770_CALLOC(n,size) \
  calloc(n,size)
#define LIB1770_FREE(p) \
  free(p)

#define LIB1770_MESSAGE(message) \
  fprintf(stderr,"Error %s in %s() at \"%s\" (%d).\n",message, \
      __func__,__FILE__,__LINE__)

#define LIB1770_GOTO(condition,message,label) do { \
  if (condition) { \
    LIB1770_MESSAGE(message); \
    goto label; \
  } \
} while (0)

///////////////////////////////////////////////////////////////////////////////
#define LIB1770_BUF_SIZE      9
#define LIB1770_LFE           3
#if defined (LIB1770_LFE) // [
#define LIB1770_MAX_CHANNELS  6
#define LIB1770_SAMPLES_SIZE  8
#else // ] [
#define LIB1770_MAX_CHANNELS  5
#endif // ]
#if defined __GNUC__ // [
#define LIB1770_DEPRECATED __attribute__ ((deprecated))
#else // ] [
#define LIB1770_DEPRECATED
#endif // ]

#define LIB1770_Q2DB(q) \
  (20.0*log10(q))
#define LIB1770_DB2Q(db) \
  (pow(10.0,0.05*(db)))

#define LIB1770_IS_DEN(x) \
    (fabs(den_tmp=(x))<1.0e-15)
#define LIB1770_DEN(x) \
    (LIB1770_IS_DEN(x)?0.0:den_tmp)

#define LIB1770_MIN(x,y) \
  ((x)<(y)?(x):(y))

#define LIB1770_SILENCE \
  (-70.0)
#define LIB1770_SILENCE_GATE \
  (pow(10.0,0.1*(0.691+LIB1770_SILENCE)))
#define LIB1770_LUFS(x) \
  (-0.691+10.0*log10(x))
#define LIB1770_LUFS_HIST(count,sum,reference) \
  ((count)?LIB1770_LUFS((sum)/((double)(count))):(double)(reference))

#define LIB1770_AGG_BLOCK_SIZE(size) \
  ((size)*sizeof(((lib1770_block_t *)NULL)->ring.wmsq[0]))

///////////////////////////////////////////////////////////////////////////////
#if defined (LIB1770_LFE) // [
typedef double lib1770_sample_t[LIB1770_SAMPLES_SIZE];
#else // ] [
typedef double lib1770_sample_t[LIB1770_MAX_CHANNELS];
#endif // ]
typedef unsigned long long lib1770_count_t;

typedef struct lib1770_biquad lib1770_biquad_t;
typedef struct lib1770_biquad_ps lib1770_biquad_ps_t;
typedef struct lib1770_pre lib1770_pre_t;
typedef struct lib1770_block lib1770_block_t;
typedef struct lib1770_bin lib1770_bin_t;
typedef struct lib1770_stats lib1770_stats_t;

///////////////////////////////////////////////////////////////////////////////
struct lib1770_biquad {
  double samplerate;
  double a1,a2;
  double b0,b1,b2;
};

struct lib1770_biquad_ps {
  double k;
  double q;
  double vb;
  double vl;
  double vh;
};

void lib1770_biquad_get_ps(const lib1770_biquad_t *biquad,
    lib1770_biquad_ps_t *ps);
lib1770_biquad_t *lib1770_biquad_requantize(lib1770_biquad_t *in,
    const lib1770_biquad_t *out);

///////////////////////////////////////////////////////////////////////////////
struct lib1770_bin {
  double db;
  double x;
  double y;
  lib1770_count_t count;
};

#define LIB1770_HIST_MIN      (-70)
#define LIB1770_HIST_MAX      (+5)
#define LIB1770_HIST_GRAIN    (100)
#define LIB1770_HIST_NBINS \
    (LIB1770_HIST_GRAIN*(LIB1770_HIST_MAX-LIB1770_HIST_MIN)+1)

struct lib1770_stats {
  lib1770_stats_t *next;

  struct {
    double wmsq;
  } max;

  struct {
    struct {
      double wmsq;            // cumulative moving average.
      lib1770_count_t count;  // number of blocks processed.
    } pass1;

#if 0 // [
    lib1770_bin_t bin[0];
#else // ] [
    lib1770_bin_t bin[1];
#endif // ]
  } hist;
};

#if ! defined (LIB1770_HIST_NBINS) // [
int lib1770_stats_create(lib1770_stats_t *stats);
void lib1770_stats_destroy(lib1770_stats_t *stats);
#endif // ]

lib1770_stats_t *lib1770_stats_new(void);
void lib1770_stats_close(lib1770_stats_t *stats);

#define LIB1770_STATS_MERGE_FIX
#if defined (LIB1770_STATS_MERGE_FIX) // [
void lib1770_stats_merge(lib1770_stats_t *lhs, const lib1770_stats_t *rhs);
#else // ] [
void lib1770_stats_merge(lib1770_stats_t *lhs, lib1770_stats_t *rhs);
#endif // ]
void lib1770_stats_add_sqs(lib1770_stats_t *stats, double wmsq);

double lib1770_stats_get_mean(lib1770_stats_t *stats, double gate);
double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper);
double lib1770_stats_get_max(lib1770_stats_t *stats);

///////////////////////////////////////////////////////////////////////////////
// ITU BS.1770 sliding block (aggregator).
struct lib1770_block {
  lib1770_block_t *next;
  lib1770_stats_t *stats;

  double gate;          // ITU BS.1770 silence gate.
  double length;        // ITU BS.1170 block length in ms
  int partition;        // ITU BS.1770 partition, e.g. 4 (75%)

  double samplerate;
  size_t overlap_size;  // depends on samplerate
  size_t block_size;    // depends on samplerate
  double scale;         // depends on block size, i.e. on samplerate

  struct {
    size_t size;        // number of blocks in ring buffer.
    size_t used;        // number of blocks used in ring buffer.
    size_t count;       // number of samples processed in front block.
    size_t offs;        // offset of front block.
    double wmsq[0];     // allocated blocks.
  } ring;
};

lib1770_block_t *lib1770_block_new(double samplerate, double ms,
    int partition);
void lib1770_block_close(lib1770_block_t *block);

void lib1770_block_add_stats(lib1770_block_t *block, lib1770_stats_t *stats);
void lib1770_block_add_sqs(lib1770_block_t *block, double wssqs);

///////////////////////////////////////////////////////////////////////////////
// ITU BS.1770 pre-filter.
struct lib1770_pre {
  lib1770_block_t *block;
  double samplerate;
  int channels;
#if defined (LIB1770_LFE) // [
  int lfe;
#endif // ]

  lib1770_biquad_t f1;
  lib1770_biquad_t f2;

  struct {
    double buf[LIB1770_MAX_CHANNELS][LIB1770_BUF_SIZE];
    int offs;
    int size;
  } ring;
};

#if defined (LIB1770_LFE) // [
lib1770_pre_t *lib1770_pre_new_lfe(double samplerate, int channels, int lfe);
lib1770_pre_t *lib1770_pre_new(double samplerate, int channels)
    LIB1770_DEPRECATED;
#else // ] [
lib1770_pre_t *lib1770_pre_new(double samplerate, int channels);
#endif // ]
void lib1770_pre_close(lib1770_pre_t *pre);

void lib1770_pre_add_block(lib1770_pre_t *pre, lib1770_block_t *block);
void lib1770_pre_add_sample(lib1770_pre_t *pre, lib1770_sample_t sample);
void lib1770_pre_flush(lib1770_pre_t *pre);

#ifdef __cplusplus
}
#endif
#endif // }
