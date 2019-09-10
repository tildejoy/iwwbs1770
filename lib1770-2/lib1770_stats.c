/*
 * lib1770_stats.c
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
#include <lib1770.h>

#if ! defined (LIB1770_HIST_NBINS) // [
int lib1770_stats_create(lib1770_stats_t *stats)
{
  double step=1.0/LIB1770_HIST_GRAIN;
  lib1770_bin_t *wp,*mp;

#if 0 // [
  stats=LIB1770_CALLOC(1,(sizeof *stats)
      +LIB1770_HIST_NBINS*(sizeof stats->hist.bin[0]));
  LIB1770_GOTO(NULL==stats,"allocating bs.1770 statistics",stats);
#endif // ]

///////////////////////////////////////////////////////////////////////////////
  stats->max.wmsq=LIB1770_SILENCE_GATE;

///////////////////////////////////////////////////////////////////////////////
  stats->hist.pass1.wmsq=0.0;
  stats->hist.pass1.count=0;

  wp=stats->hist.bin;
  mp=wp+LIB1770_HIST_NBINS;

  while (wp<mp) {
    size_t i=wp-stats->hist.bin;
    double db=step*i+LIB1770_HIST_MIN;
    double wsmq=pow(10.0,0.1*(0.691+db));

    wp->db=db;
    wp->x=wsmq;
    wp->y=0.0;
    wp->count=0;

    if (0<i)
      wp[-1].y=wsmq;

    ++wp;
  }

  return 0;
#if 0 // [
stats:
  return NULL;
#endif // ]
}

void lib1770_stats_destroy(lib1770_stats_t *stats)
{
  (void)stats;
}
#endif // ]

lib1770_stats_t *lib1770_stats_new(void)
{
  lib1770_stats_t *stats;
#if defined (LIB1770_HIST_NBINS) // [
  double step=1.0/LIB1770_HIST_GRAIN;
  lib1770_bin_t *wp,*mp;
#endif // ]

  stats=LIB1770_CALLOC(1,(sizeof *stats)
      +LIB1770_HIST_NBINS*(sizeof stats->hist.bin[0]));
  LIB1770_GOTO(NULL==stats,"allocating bs.1770 statistics",stats);

#if defined (LIB1770_HIST_NBINS) // [
///////////////////////////////////////////////////////////////////////////////
  stats->max.wmsq=LIB1770_SILENCE_GATE;

///////////////////////////////////////////////////////////////////////////////
  stats->hist.pass1.wmsq=0.0;
  stats->hist.pass1.count=0;

  wp=stats->hist.bin;
  mp=wp+LIB1770_HIST_NBINS;

  while (wp<mp) {
    size_t i=wp-stats->hist.bin;
    double db=step*i+LIB1770_HIST_MIN;
    double wsmq=pow(10.0,0.1*(0.691+db));

    wp->db=db;
    wp->x=wsmq;
    wp->y=0.0;
    wp->count=0;

    if (0<i)
      wp[-1].y=wsmq;

    ++wp;
  }

  return stats;
#else // ] [
  if (lib1770_stats_create(stats)<0)
    goto create;

  return stats;
create:
  LIB1770_FREE(stats);
#endif // ]
stats:
  return NULL;
}

void lib1770_stats_close(lib1770_stats_t *stats)
{
#if ! defined (LIB1770_HIST_NBINS) // [
  lib1770_stats_destroy(stats);
#endif // ]
  LIB1770_FREE(stats);
}

#if defined (LIB1770_STATS_MERGE_FIX) // [
void lib1770_stats_merge(lib1770_stats_t *lhs, const lib1770_stats_t *rhs)
#else // ] [
void lib1770_stats_merge(lib1770_stats_t *lhs, lib1770_stats_t *rhs)
#endif // ]
{
  // m1=(sum_n c_n)/n
  // m2=(sum_m c_m)/m
  //
  // n*m1=sum_n c_n
  // m*m2=sum_m c_m
  //
  // ((sum_n c_n) + (sum_m c_n)) / (m+n)
  //     = n/(m+n) m1 + m/(m+n) m2
  lib1770_count_t count;
  double q1,q2;
#if defined (LIB1770_STATS_MERGE_FIX) // [
  lib1770_bin_t *bin1,*mp;
  const lib1770_bin_t *bin2;
#else // ] [
  lib1770_bin_t *bin1,*bin2,*mp;
#endif // ]

  if (lhs->max.wmsq<rhs->max.wmsq)
    lhs->max.wmsq=rhs->max.wmsq;

  count=lhs->hist.pass1.count+rhs->hist.pass1.count;

  if (0ull<count) {
    q1=(double)lhs->hist.pass1.count/count;
    q2=(double)rhs->hist.pass1.count/count;
#if defined (LIB1770_STATS_MERGE_FIX) // [
    lhs->hist.pass1.count=count;
#endif // ]
    lhs->hist.pass1.wmsq=q1*lhs->hist.pass1.wmsq+q2*rhs->hist.pass1.wmsq;
    bin1=lhs->hist.bin;
    bin2=rhs->hist.bin;
    mp=bin1+LIB1770_HIST_NBINS;

    while (bin1<mp)
      (bin1++)->count+=(bin2++)->count;
  }
}

static int lib1770_bin_cmp(const void *key, const void *bin)
{
  if (*(const double *)key<((const lib1770_bin_t *)bin)->x)
    return -1;
  else if (0==((const lib1770_bin_t *)bin)->y)
    return 0;
  else if (((const lib1770_bin_t *)bin)->y<=*(const double *)key)
    return 1;
  else
    return 0;
}

void lib1770_stats_add_sqs(lib1770_stats_t *stats, double wmsq)
{
  lib1770_bin_t *bin;

///////////////////////////////////////////////////////////////////////////////
  if (stats->max.wmsq<wmsq)
    stats->max.wmsq=wmsq;

///////////////////////////////////////////////////////////////////////////////
  bin=bsearch(&wmsq,stats->hist.bin,LIB1770_HIST_NBINS,
      sizeof stats->hist.bin[0],lib1770_bin_cmp);

  if (NULL!=bin) {
    // cumulative moving average.
    // https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
#if 1 // {
	//                       x (n) - CMA (n)
	// CMA (n+1) = CMA (n) + ---------------
	//                          n + 1
    stats->hist.pass1.wmsq+=(wmsq-stats->hist.pass1.wmsq)
        /(double)(++stats->hist.pass1.count);
#else // } {
	//             x + n * CMA (n)
    // CMA (n+1) = ---------------
	//                n + 1
	//
	//           = x / (n + 1) + CMA (n) * n / (n+1)
	//
	double n=stats->hist.pass1.count;
	double m=n+1;

    stats->hist.pass1.wmsq*=n/m;
    stats->hist.pass1.wmsq+=wmsq/m;
	++stats->hist.pass1.count;
#endif // }
    ++bin->count;
  }
}

///////////////////////////////////////////////////////////////////////////////
double lib1770_stats_get_max(lib1770_stats_t *stats)
{
  return LIB1770_LUFS(stats->max.wmsq);
}

///////////////////////////////////////////////////////////////////////////////
double lib1770_stats_get_mean(lib1770_stats_t *stats, double gate)
{
  const lib1770_bin_t *rp,*mp;
  double wmsq=0.0;
  lib1770_count_t count=0ull;

  rp=stats->hist.bin;
  mp=rp+LIB1770_HIST_NBINS;
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);

  while (rp<mp) {
    if (0ull<rp->count&&gate<rp->x) {
      wmsq+=(double)rp->count*rp->x;
      count+=rp->count;
    }

    ++rp;
  }

  return LIB1770_LUFS_HIST(count,wmsq,LIB1770_SILENCE);
}

double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper)
{
  const lib1770_bin_t *rp,*mp;
  lib1770_count_t count=0ull;

  rp=stats->hist.bin;
  mp=rp+LIB1770_HIST_NBINS;
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);

  while (rp<mp) {
    if (0ull<rp->count&&gate<rp->x)
      count+=rp->count;

    ++rp;
  }

  if (lower>upper) {
    double tmp=lower;

    lower=upper;
    upper=tmp;
  }

  if (lower<0.0)
    lower=0.0;

  if (1.0<upper)
    upper=1.0;

  if (0ull<count) {
    lib1770_count_t lower_count=count*lower;
    lib1770_count_t upper_count=count*upper;
    lib1770_count_t prev_count=-1;
    double min=0.0;
    double max=0.0;

    rp=stats->hist.bin;
    count=0ull;

    while (rp<mp) {
      if (gate<rp->x) {
        count+=rp->count;

        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
      }

      ++rp;
    }

    return max-min;
  }
  else
    return 0.0;
}
