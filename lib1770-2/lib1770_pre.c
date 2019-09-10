/*
 * lib1770_pre.c
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

///////////////////////////////////////////////////////////////////////////////
//#if defined (_MSC_VER) // {
#define lib1770_get(offs,i) \
  ((offs)+(i)<0?LIB1770_BUF_SIZE+(offs)+(i):(offs)+(i))
//#else // } {
//inline int lib1770_get(int offs, int i)
//{
//  int j=offs+i;
//
//  return j<0?LIB1770_BUF_SIZE+j:j;
//}
//#endif // }

#define LIB1770_GET(buf,offs,i) \
    ((buf)[lib1770_get(offs,i)])
#define LIB1770_GETX(buf,offs,i) \
    LIB1770_GET(buf,offs,i)
#define LIB1770_GETY(buf,offs,i) \
    LIB1770_GET(buf,(offs)-6,i)
#define LIB1770_GETZ(buf,offs,i) \
    LIB1770_GET(buf,(offs)-3,i)

///////////////////////////////////////////////////////////////////////////////
static double lib1770_g[LIB1770_MAX_CHANNELS]={
  1.0,
  1.0,
  1.0,
  1.41,
  1.41
};

static const lib1770_biquad_t *lib1770_f1_48000(void)
{
  static lib1770_biquad_t biquad;

  if (0.0==biquad.samplerate) {
    biquad.samplerate=48000;
    biquad.a1=-1.69065929318241;
    biquad.a2=0.73248077421585;
    biquad.b0=1.53512485958697;
    biquad.b1=-2.69169618940638;
    biquad.b2=1.19839281085285;
  }

  return &biquad;
}

static const lib1770_biquad_t *lib1770_f2_48000(void)
{
  static lib1770_biquad_t biquad;

  if (0.0==biquad.samplerate) {
    biquad.samplerate=48000;
    biquad.a1=-1.99004745483398;
    biquad.a2=0.99007225036621;
    biquad.b0=1.0;
    biquad.b1=-2.0;
    biquad.b2=1.0;
  }

  return &biquad;
}

///////////////////////////////////////////////////////////////////////////////
#if defined (LIB1770_LFE) // [
lib1770_pre_t *lib1770_pre_new_lfe(double samplerate, int channels, int lfe)
#else // ] [
lib1770_pre_t *lib1770_pre_new(double samplerate, int channels)
#endif // ]
{
  lib1770_pre_t *pre;
  int i,ch;

  pre=LIB1770_CALLOC(1,sizeof *pre);
  LIB1770_GOTO(NULL==pre,"allocation bs.1770 pre-filter",epre);

  pre->block=NULL;

  // set the sample frequency.
  pre->samplerate=samplerate;
  // set the number of channals.
  pre->channels=channels;
#if defined (LIB1770_LFE) // [
  // set the lfe channal.
  if (LIB1770_LFE<lfe) {
    PBU_DVMESSAGE("lfe overflow: %d (%d)",lfe,LIB1770_LFE);
    goto elfe;
  }

  pre->lfe=lfe;
#endif // ]

  // requantize the f1-filter according to the sample frequency.
  pre->f1.samplerate=samplerate;
  lib1770_biquad_requantize(&pre->f1,lib1770_f1_48000());

  // requantize the f2-filter according to the sample frequency.
  pre->f2.samplerate=samplerate;
  lib1770_biquad_requantize(&pre->f2,lib1770_f2_48000());

  // initialize the pre buffer.
  for (i=0,ch=0;i<LIB1770_MIN(channels,LIB1770_MAX_CHANNELS);++i) {
#if defined (LIB1770_LFE) // [
		// filter out the lfe channel.
		if (i==lfe)
			continue;
#endif // ]

    LIB1770_GETX(pre->ring.buf[ch],0,0)=0.0;
		++ch;
	}

  pre->ring.offs=1;
  pre->ring.size=pre->ring.offs;

  return pre;
  //LIB1770_FREE(pre);
epre:
#if defined (LIB1770_LFE) // [
elfe:
#endif // ]
  return NULL;
}

#if defined (LIB1770_LFE) // [
lib1770_pre_t *lib1770_pre_new(double samplerate, int channels)
{
  return lib1770_pre_new_lfe(samplerate,channels,-1);
}
#endif // ]

void lib1770_pre_close(lib1770_pre_t *pre)
{
  LIB1770_FREE(pre);
}

void lib1770_pre_add_block(lib1770_pre_t *pre, lib1770_block_t *block)
{
  block->next=pre->block;
  pre->block=block;
}

void lib1770_pre_add_sample(lib1770_pre_t *pre, lib1770_sample_t sample)
{
  lib1770_biquad_t *f1=&pre->f1;
  lib1770_biquad_t *f2=&pre->f2;
  double wssqs=0.0;
  double *g=lib1770_g;
  int channels=pre->channels;
#if defined (LIB1770_LFE) // [
  int lfe=pre->lfe;
#endif // ]
  int offs=pre->ring.offs;
  int size=pre->ring.size;
  int i,ch;
  lib1770_block_t *block;
  double den_tmp;
  double *buf;
  double x;

  for (i=0,ch=0;i<LIB1770_MIN(channels,LIB1770_MAX_CHANNELS);++i) {
#if defined (LIB1770_LFE) // [
		// filter out the lfe channel.
		if (lfe==i)
			continue;
#endif // ]

    buf=pre->ring.buf[ch];
    x=LIB1770_GETX(buf,offs,0)=LIB1770_DEN(sample[ch]);

    if (1<size) {
      double y=LIB1770_GETY(buf,offs,0)=LIB1770_DEN(f1->b0*x
        +f1->b1*LIB1770_GETX(buf,offs,-1)+f1->b2*LIB1770_GETX(buf,offs,-2)
        -f1->a1*LIB1770_GETY(buf,offs,-1)-f1->a2*LIB1770_GETY(buf,offs,-2))
        ;
      double z=LIB1770_GETZ(buf,offs,0)=LIB1770_DEN(f2->b0*y
        +f2->b1*LIB1770_GETY(buf,offs,-1)+f2->b2*LIB1770_GETY(buf,offs,-2)
        -f2->a1*LIB1770_GETZ(buf,offs,-1)-f2->a2*LIB1770_GETZ(buf,offs,-2))
        ;
      wssqs+=(*g++)*z*z;
      ++buf;
    }

		++ch;
  }

  for (block=pre->block;NULL!=block;block=block->next)
    lib1770_block_add_sqs(block,wssqs);

  if (size<2)
    ++pre->ring.size;

  if (++pre->ring.offs==LIB1770_BUF_SIZE)
    pre->ring.offs=0;
}

void lib1770_pre_flush(lib1770_pre_t *pre)
{
  int channels=pre->channels;
#if defined (LIB1770_LFE) // [
  int lfe=pre->lfe;
#endif // ]
  lib1770_sample_t sample;
  int i,ch;

  if (1<pre->ring.size) {
  	for (i=0,ch=0;i<LIB1770_MIN(channels,LIB1770_MAX_CHANNELS);++i) {
#if defined (LIB1770_LFE) // [
			// filter out the lfe channel.
			if (lfe==i)
				continue;
#endif // ]

      sample[ch]=0.0;
			++ch;
		}

    lib1770_pre_add_sample(pre,sample);
  }
}
