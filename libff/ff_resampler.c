/*
 * ff_resampler.c
 *
 * Copyright (C) 2019 Peter Belkner <pbelkner@users.sf.net>
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
#include <ff.h>

///////////////////////////////////////////////////////////////////////////////
int ff_resampler_create(ff_resampler_t *res,
    const AVCodecParameters *ocodecpar,
    const AVCodecParameters *icodecpar)
{
  int64_t ochannel_layout=ocodecpar->channel_layout;

  /////////////////////////////////////////////////////////////////////////////
  if (!ochannel_layout||!icodecpar->channel_layout) {
    DVMESSAGE("invalid channel layout: output=%I64d input=%I64d",
        ochannel_layout,icodecpar->channel_layout);
    goto e_channel_layout;
  }

  /////////////////////////////////////////////////////////////////////////////
  res->ctx=swr_alloc_set_opts(
      NULL,                       // struct SwrContext *s,
      // [
      ochannel_layout,            // int64_t out_ch_layout,
      ocodecpar->format,          // enum AVSampleFormat out_sample_fmt,
      ocodecpar->sample_rate,     // int out_sample_rate,
      // ] [
      icodecpar->channel_layout,  // int64_t in_ch_layout,
      icodecpar->format,          // enum AVSampleFormat in_sample_fmt,
      icodecpar->sample_rate,     // int in_sample_rate,
      // ]
      0,                          // int log_obgset,
      NULL);                      // void *log_ctx

  if (!res->ctx) {
    DMESSAGE("allocating context");
    goto e_context;
  }

  /////////////////////////////////////////////////////////////////////////////
  res->frame=av_frame_alloc();

  if (!res->frame) {
    DMESSAGE("allocating frame");
    goto e_frame;
  }

  res->frame->channel_layout=ochannel_layout;
  res->frame->channels=av_get_channel_layout_nb_channels(ochannel_layout);
  res->frame->format=ocodecpar->format;
  res->frame->sample_rate=ocodecpar->sample_rate;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  av_frame_free(&res->frame);
e_frame:
  swr_close(res->ctx);
  swr_free(&res->ctx);
e_context:
e_channel_layout:
  return -1;
}

void resampler_destroy(ff_resampler_t *res)
{
  av_frame_free(&res->frame);
  swr_close(res->ctx);
  swr_free(&res->ctx);
}

int resampler_apply(ff_resampler_t *res, AVFrame *frame)
{
  int err;

  /////////////////////////////////////////////////////////////////////////////
  err=swr_convert_frame(res->ctx,res->frame,frame);

  if (err<0) {
    DVMESSAGE("converting frame: %s (%d)",av_err2str(err),err);
    goto econvert;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
econvert:
  return -1;
}
