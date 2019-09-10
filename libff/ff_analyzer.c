/*
 * ff_analyzer.c
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
#define FF_ANALYZER_CREATE_OCHANNELS
int ff_analyzer_create(ff_analyzer_t *a, ff_inout_t *in)
{
  ff_input_callback_t *cb=in->cb.in;
  void *data=in->cb.data;
  int upsample=cb&&cb->upsample?cb->upsample(data):-1;
  const ff_param_decode_t *(*decode)(const void *)=cb?cb->decode:NULL;
  int64_t channel_layout=decode?decode(data)->request.channel_layout:-1ll;
  int (*create)(void *, const AVCodecParameters *)=cb?cb->stats.create:NULL;
  AVCodecParameters *icodecpar,ocodecpar;

  /////////////////////////////////////////////////////////////////////////////
  if (in->ai<0) {
    DMESSAGE("input not initialized");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  icodecpar=in->fmt.ctx->streams[in->ai]->codecpar;

  if (!icodecpar->channel_layout) {
    DMESSAGE("missing input channel layout");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!channel_layout) {
    DMESSAGE("missing output channel layout");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!in->audio.ctx) {
    DMESSAGE("audio decoder not opened");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  a->state=FF_ANALYZER_DECODER_SEND_PACKET;
  a->in=in;

  /////////////////////////////////////////////////////////////////////////////
//DVWRITELN("%d",icodecpar->channels);
  if (0ll<channel_layout&&channel_layout!=(int)icodecpar->channel_layout) {
    ocodecpar=*icodecpar;
    ocodecpar.channel_layout=channel_layout;
#if defined (FF_ANALYZER_CREATE_OCHANNELS) // [
DVWRITELN("%I64d",ocodecpar.channel_layout);
    ocodecpar.channels
        =av_get_channel_layout_nb_channels(ocodecpar.channel_layout);
#endif // ]

    ///////////////////////////////////////////////////////////////////////////
    if (ff_resampler_create(&a->normalizer,&ocodecpar,icodecpar)<0) {
      DMESSAGE("creating normalizer");
      goto e_normalizer;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (create&&create(data,&ocodecpar)<0) {
      DMESSAGE("creating statistics");
      goto e_stats;
    }
  }
  else {
    ///////////////////////////////////////////////////////////////////////////
    a->normalizer.ctx=NULL;

    ///////////////////////////////////////////////////////////////////////////
    if (create&&create(data,icodecpar)<0) {
      DMESSAGE("creating statistics");
      goto e_stats;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  a->pkt=av_packet_alloc();

  if (!a->pkt) {
    DMESSAGE("allocating packet");
    goto e_packet;
  }

  /////////////////////////////////////////////////////////////////////////////
  a->frame=av_frame_alloc();

  if (!a->frame) {
    DMESSAGE("allocating frame");
    goto e_frame;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (upsample<1)
    a->upsampler.ctx=NULL;
  else {
    ocodecpar=*icodecpar;
    ocodecpar.sample_rate*=upsample;

    if (ff_resampler_create(&a->upsampler,&ocodecpar,icodecpar)<0) {
      DMESSAGE("creating upsampler");
      goto e_upsampler;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (a->upsampler.ctx)
    resampler_destroy(&a->upsampler);
e_upsampler:
  av_frame_free(&a->frame);
e_frame:
  av_packet_free(&a->pkt);
e_packet:
  if (cb&&cb->stats.destroy)
    cb->stats.destroy(data);
e_stats:
  if (a->normalizer.ctx)
   resampler_destroy(&a->normalizer);
e_normalizer:
e_args:
  return -1;
}

void ff_analyzer_destroy(ff_analyzer_t *a, int destroy_stats)
{
  ff_input_callback_t *cb=a->in->cb.in;
  void *data=a->in->cb.data;

  if (a->upsampler.ctx)
    resampler_destroy(&a->upsampler);

  av_frame_free(&a->frame);
  av_packet_free(&a->pkt);

  if (destroy_stats&&cb&&cb->stats.destroy)
    cb->stats.destroy(data);

  if (a->normalizer.ctx)
   resampler_destroy(&a->normalizer);
}

///////////////////////////////////////////////////////////////////////////////
static int ff_analyzer_process_frame(ff_analyzer_t *a, AVFrame *frame)
{
  ff_input_callback_t *cb=a->in->cb.in;
  void *data=a->in->cb.data;
  int (*add)(void *, int, AVFrame *)=cb?cb->stats.add:NULL;
  int err;

  if (a->normalizer.ctx) {
    err=resampler_apply(&a->normalizer,frame);

    if (err<0) {
      DVMESSAGE("applying normalizer: %s (%d)",av_err2str(err),err);
      goto exit;
    }

    if (add) {
      err=add(data,0,a->normalizer.frame);

      if (err<0) {
        DVMESSAGE("adding normalized statistics: %s (%d)",av_err2str(err),err);
        goto exit;
      }
    }
  }
  else if (add) {
    err=add(data,0,frame);

    if (err<0) {
      DVMESSAGE("adding statistics: %s (%d)",av_err2str(err),err);
      goto exit;
    }
  }

  if (a->upsampler.ctx) {
    err=resampler_apply(&a->upsampler,frame);

    if (err<0) {
      DVMESSAGE("applying upsampler: %s (%d)",av_err2str(err),err);
      goto exit;
    }

    if (add) {
      err=add(data,1,a->upsampler.frame);

      if (err<0) {
        DVMESSAGE("adding upsampled statistics: %s (%d)",av_err2str(err),err);
        goto exit;
      }
    }
  }

  err=0;
exit:
  return err;
}

static int ff_analyzer_send_packet(ff_analyzer_t *a, AVPacket *pkt)
{
  int err=-1;

  for (;;) {
    switch (a->state) {
    case FF_ANALYZER_DECODER_SEND_PACKET:
      err=avcodec_send_packet(a->in->audio.ctx,pkt);

#if defined (FF_PACKET_UNREF) // [
      if (pkt)
        av_packet_unref(pkt);
#endif // ]

      switch (err) {
      case AVERROR_EOF:
        goto e_loop;
      case AVERROR_INVALIDDATA:
        // Invalid data found when processing input.
        // No respective documentation found. May occur at the end of MP3.
        // Intentional fall-through.
        continue;
      case 0:
        // 0 on success
        a->state=FF_ANALYZER_DECODER_RECEIVE_FRAME;
        continue;
      default:
        DVMESSAGE("sending packet: %s (%d)",av_err2str(err),err);
        return err;
      }
    case FF_ANALYZER_DECODER_RECEIVE_FRAME:
      err=avcodec_receive_frame(a->in->audio.ctx,a->frame);

      switch (err) {
      case 0:
        // 0: success, a frame was returned
        err=ff_analyzer_process_frame(a,a->frame);
#if defined (FF_FRAME_UNREF) // [
        av_frame_unref(a->frame);
#endif // ]

        if (err<0) {
          DVMESSAGE("processing frame: %s (%d)",av_err2str(err),err);
          goto e_loop;
        }

        continue;
      case AVERROR_EOF:
        // we need to flush the encoder.
        a->state=FF_ANALYZER_DECODER_SEND_PACKET;
        return err;
      case AVERROR(EAGAIN):
        // AVERROR(EAGAIN): output is not available in this state - user must
        // try to send new input
        // read the next packet.
        a->state=FF_ANALYZER_DECODER_SEND_PACKET;
        return err;
      default:
        DVMESSAGE("receiving frame: %s (%d)",av_err2str(err),err);
        return err;
      }
    default:
      DMESSAGE("unexpected state");
      goto e_loop;
    }
  }

  return 0;
e_loop:
  return err;
}

int ff_analyzer_loop(ff_analyzer_t *a)
{
  AVFormatContext *ctx=a->in->fmt.ctx;
  AVPacket *pkt=a->pkt;
  int err;
read:
  err=av_read_frame(ctx,pkt);

  if (err<0)
    goto eof;
  else if ((int)ctx->nb_streams<=pkt->stream_index) {
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(a->pkt);
#endif // ]
    goto read;
  }

  if (ff_input_progress(a->in,pkt)<0)
    goto eof;

  if (a->in->ai!=a->pkt->stream_index) {
    // a packet from a stream we're not interested in has to be skipped.
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(a->pkt);
#endif // ]
    goto read;
  }

  // an audio packet has been read which has to be decoded.
  // the packet is unrefed by analyzer_send_packet().
  err=ff_analyzer_send_packet(a,a->pkt);

  if (FF_ANALYZER_DECODER_SEND_PACKET==a->state) {
    switch (err) {
    case AVERROR(EAGAIN):
      goto read;
    case AVERROR_EOF:
      goto eof;
    default:
      DVMESSAGE("decoding frame: %s (%d)",av_err2str(err),err);
      goto e_loop;
    }
  }
eof:
  /////////////////////////////////////////////////////////////////////////////
  // we need to flush the decoder.
  err=ff_analyzer_send_packet(a,NULL);

  if (err<0&&FF_ANALYZER_DECODER_SEND_PACKET<a->state) {
    DVMESSAGE("decoding frame: %s (%d)",av_err2str(err),err);
    goto e_loop;
  }

  /////////////////////////////////////////////////////////////////////////////
  ff_printer_flush(a->in->printer);

  /////////////////////////////////////////////////////////////////////////////
  return 0;
e_loop:
  return err;
}
