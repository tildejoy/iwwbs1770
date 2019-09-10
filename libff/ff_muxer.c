/*
 * ff_muxer.c
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
//#define FF_DEBUG_LOCAL
#if defined (FF_DEBUG_LOCAL) // [
#define DDMARKLN() DMARKLN()
#define DDWRITELN(line) DWRITELN(line)
#define DDVWRITELN(format,...) DVWRITELN(format,__VA_ARGS__)
#else // ] [
#define DDMARKLN()
#define DDWRITELN(line)
#define DDVWRITELN(format,...)
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int ff_muxer_create(ff_muxer_t *m, ff_inout_t *in, ff_inout_t *out,
    const char *filter)
{
  AVStream *istream=in->fmt.ctx->streams[in->ai];
  AVStream *ostream=out->fmt.ctx->streams[out->ai];

  /////////////////////////////////////////////////////////////////////////////
  m->state=FF_MUXER_DECODER_SEND_PACKET;
  m->in=in;
  m->out=out;

  /////////////////////////////////////////////////////////////////////////////
  m->pkt=av_packet_alloc();

  if (!m->pkt) {
    DMESSAGE("allocating packet");
    goto e_packet;
  }

  if (in->audio.ctx) {
    ///////////////////////////////////////////////////////////////////////////
    m->frame=av_frame_alloc();

    if (!m->frame) {
      DMESSAGE("allocating frame");
      goto e_frame;
    }
  }
  else
    m->frame=NULL;

  if (filter) {
    ///////////////////////////////////////////////////////////////////////////
    if (ff_filter_create(&m->filter,ostream->codecpar,istream->codecpar,
        istream->time_base,filter)<0) {
      DMESSAGE("creating filter");
      goto e_filter;
    }
  }
  else {
    ///////////////////////////////////////////////////////////////////////////
    m->filter.graph=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  m->audio.ts=0ll;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (m->filter.graph)
    ff_filter_destroy(&m->filter);
e_filter:
  if (m->frame)
    av_frame_free(&m->frame);
e_frame:
  av_packet_free(&m->pkt);
e_packet:
  return -1;
}

void ff_muxer_destroy(ff_muxer_t *m)
{
  if (m->filter.graph)
    ff_filter_destroy(&m->filter);

  if (m->frame)
    av_frame_free(&m->frame);

  av_packet_free(&m->pkt);
}

///////////////////////////////////////////////////////////////////////////////
static int ff_muxer_mux_audio_frame(ff_muxer_t *m, AVPacket *pkt)
{
  // cf. "<ffmpeg>/doc/examples/muxing.c".
  ff_audio_t *audio=&m->out->audio;
  AVStream *ostream=m->out->fmt.ctx->streams[m->out->ai];

  /////////////////////////////////////////////////////////////////////////////
  pkt->dts=m->audio.ts;
  pkt->pts=m->audio.ts;
  m->audio.ts+=pkt->duration;

  /////////////////////////////////////////////////////////////////////////////
  // rescale output packet timestamp values from codec to stream timebase.
  av_packet_rescale_ts(pkt,audio->ctx->time_base,ostream->time_base);
  pkt->stream_index=m->out->ai;

  /////////////////////////////////////////////////////////////////////////////
  // Write the compressed frame to the media file.
  return av_interleaved_write_frame(m->out->fmt.ctx,pkt);
}

//#define FF_SIDE_DATA
#if defined (FF_SIDE_DATA) // [
static int ff_muxer_remux_frame(ff_muxer_t *m, AVPacket *pkt, int stream_index)
{
  int err=-1;
  AVPacket opkt={ 0 };
  AVStream *istream,*ostream;

  av_init_packet(&opkt);

  istream=m->in->fmt.ctx->streams[pkt->stream_index];
  opkt.stream_index=stream_index;
  ostream=m->out->fmt.ctx->streams[opkt.stream_index];

  opkt.pts=av_rescale_q_rnd(pkt->pts,istream->time_base,
      ostream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
  opkt.dts=av_rescale_q_rnd(pkt->dts,istream->time_base,
      ostream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);

  opkt.duration=av_rescale_q(pkt->duration,istream->time_base,
      ostream->time_base);
  opkt.flags=pkt->flags;

  if (pkt->buf) {
    opkt.buf=av_buffer_ref(pkt->buf);

    if (!opkt.buf) {
      DMESSAGE("referencing buffer");
      goto e_ref;
    }
  }

  opkt.data=pkt->data;
  opkt.size=pkt->size;
#if 0 // [
  av_copy_packet_side_data(&opkt,pkt);
#else // ] [
  if (av_packet_copy_props(&opkt,pkt)<0) {
    DMESSAGE("copying properties");
    goto e_props;
  }
#endif // ]
  //opkt.pos=-1;

  if (av_interleaved_write_frame(m->out->fmt.ctx,&opkt)<0) {
    DMESSAGE("writing");
    goto e_write;
  }

  err=0;
//cleanup:
e_write:
e_props:
e_ref:
  return err;
}
#else // ] [
static int ff_muxer_remux_frame(ff_muxer_t *m, AVPacket *pkt, int stream_index)
{
  // cf. "<ffmpeg>/doc/examples/remuxing.c".
  AVStream *istream,*ostream;

  istream=m->in->fmt.ctx->streams[pkt->stream_index];
  pkt->stream_index=stream_index;
  ostream=m->out->fmt.ctx->streams[pkt->stream_index];

  pkt->pts=av_rescale_q_rnd(pkt->pts,istream->time_base,
      ostream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
  pkt->dts=av_rescale_q_rnd(pkt->dts,istream->time_base,
      ostream->time_base,AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);

  pkt->duration=av_rescale_q(pkt->duration,istream->time_base,
      ostream->time_base);
  pkt->pos=-1;

  return av_interleaved_write_frame(m->out->fmt.ctx,pkt);
}
#endif // ]

static int ff_muxer_send_packet(ff_muxer_t *m, AVPacket *pkt)
{
  AVFrame *frame=NULL;
  int err;

  for (;;) {
    switch (m->state) {
    case FF_MUXER_DECODER_SEND_PACKET:
DDVWRITELN("FF_MUXER_DECODER_SEND_PACKET: %d",pkt?pkt->size:-1);
      err=avcodec_send_packet(m->in->audio.ctx,pkt);
DDVWRITELN("FF_MUXER_DECODER_SEND_PACKET: %d",err);

#if defined (FF_PACKET_UNREF) // [
      if (pkt)
        av_packet_unref(pkt);
#endif // ]

      switch (err) {
      case AVERROR_INVALIDDATA:
        // Invalid data found when processing input.
        // No respective documentation found. May occur at the end of MP3.
        // Intentional fall-through.
      case 0:
        // 0 on success
        m->state=FF_MUXER_DECODER_RECEIVE_FRAME;
        continue;
      default:
        DVMESSAGE("sending packet: %s (%d)",av_err2str(err),err);
        return err;
      }
    case FF_MUXER_DECODER_RECEIVE_FRAME:
DDWRITELN("FF_MUXER_DECODER_RECEIVE_FRAME");
      err=avcodec_receive_frame(m->in->audio.ctx,m->frame);
DDVWRITELN("FF_MUXER_DECODER_RECEIVE_FRAME: %d (%d)",
    m->frame->nb_samples,err);

      switch (err) {
      case 0:
        // 0: success, a frame was returned
        frame=m->frame;

        if (m->filter.graph)
          m->state=FF_MUXER_FILTER_SEND_FRAME;
        else
          m->state=FF_MUXER_ENCODER_SEND_FRAME;

        continue;
      case AVERROR_EOF:
        // we need to flush the encoder.
        frame=NULL;

        if (m->filter.graph)
          m->state=FF_MUXER_FILTER_SEND_FRAME;
        else
          m->state=FF_MUXER_ENCODER_SEND_FRAME;

        continue;
      case AVERROR(EAGAIN):
        // AVERROR(EAGAIN): output is not available in this state - user must
        // try to send new input
        // read the next packet.
        m->state=FF_MUXER_DECODER_SEND_PACKET;
        return err;
      default:
        DVMESSAGE("receiving frame: %s (%d)",av_err2str(err),err);
        return err;
      }
    case FF_MUXER_FILTER_SEND_FRAME:
DDWRITELN("FF_MUXER_FILTER_RECEIVE_FRAME");
      err=ff_filter_send_frame(&m->filter,frame);
DDVWRITELN("FF_MUXER_FILTER_SEND_FRAME: %d (%d)",
    frame?frame->nb_samples:-1,err);

#if defined (FF_FRAME_UNREF) // [
      if (frame==m->frame)
        av_frame_unref(frame);
#endif // ]

      if (err<0) {
        DVMESSAGE("sending frame to filter: %s (%d)",av_err2str(err),err);
        return err;
      }

      m->state=FF_MUXER_FILTER_RECEIVE_FRAME;
      continue;
    case FF_MUXER_FILTER_RECEIVE_FRAME:
DDWRITELN("FF_MUXER_FILTER_RECEIVE_FRAME");
      err=ff_filter_receive_frame(&m->filter,m->frame);
DDVWRITELN("FF_MUXER_FILTER_RECEIVE_FRAME: %d (%d)",
    m->frame->nb_samples,err);

      switch (err) {
        case 0:
          frame=m->frame;
          m->state=FF_MUXER_ENCODER_SEND_FRAME;
          continue;
        case AVERROR(EAGAIN):
          m->state=FF_MUXER_DECODER_RECEIVE_FRAME;
          continue;
        case AVERROR_EOF:
          // we should flush the encoder.
          frame=NULL;
          m->state=FF_MUXER_ENCODER_SEND_FRAME;
          continue;
        default:
          DVMESSAGE("filter receiving frame: %s (%d)",av_err2str(err),err);
          return err;
      }
    case FF_MUXER_ENCODER_SEND_FRAME:
      if (frame&&m->out->audio.ctx->frame_size<frame->nb_samples) {
        DVMESSAGE("frame too large: frame_size:%d<nb_samples:%d",
            m->out->audio.ctx->frame_size,frame->nb_samples);
        return AVERROR(EAGAIN);
      }

DDWRITELN("FF_MUXER_ENCODER_SEND_FRAME");
      err=avcodec_send_frame(m->out->audio.ctx,frame);
DDVWRITELN("FF_MUXER_ENCODER_SEND_FRAME: %d (%d)",
    frame?frame->nb_samples:-1,err);

      if (err<0) {
        DVMESSAGE("encoder sending frame: %s (%d)",av_err2str(err),err);
        return err;
      }
      else {
        m->state=FF_MUXER_ENCODER_RECEIVE_PACKET;
        continue;
      }

#if defined (FF_FRAME_UNREF) // [
      if (frame==m->frame)
        av_frame_unref(frame);
#endif // ]

      switch (err) {
      case 0:
        m->state=FF_MUXER_ENCODER_RECEIVE_PACKET;
        continue;
      default:
        DVMESSAGE("sending frame: %s (%d)",av_err2str(err),err);
        return err;
      }
    case FF_MUXER_ENCODER_RECEIVE_PACKET:
DDWRITELN("FF_MUXER_ENCODER_RECEIVE_PACKET");
      err=avcodec_receive_packet(m->out->audio.ctx,m->pkt);
DDVWRITELN("FF_MUXER_ENCODER_RECEIVE_PACKET: %d (%d)",m->pkt->size,err);

      switch (err) {
      case 0:
        err=ff_muxer_mux_audio_frame(m,m->pkt);
        fflush(stderr);
#if defined (FF_PACKET_UNREF) // [
        av_packet_unref(m->pkt);
#endif // ]

        if (err<0) {
          DMESSAGE("writing packet");
          return err;
        }

        continue;
      case AVERROR_EOF:
#if 0 // [
        // not an error just THE END.
        // everything (decoder and encoder) is flushed.
        DVMESSAGE("sending frame: %s (%d)",av_err2str(err),err);
#else // ] [
        m->state=FF_MUXER_DECODER_SEND_PACKET;
#endif // ]
        return err;
      case AVERROR(EAGAIN):
        m->state=FF_MUXER_DECODER_RECEIVE_FRAME;
        continue;
      default:
        DVMESSAGE("receiving packet: %s (%d)",av_err2str(err),err);
        return err;
      }
    default:
      DMESSAGE("unexpected state");
      return -1;
    }
  }
}

int ff_muxer_loop(ff_muxer_t *m)
{
  int err;
read:
  err=av_read_frame(m->in->fmt.ctx,m->pkt);

  if (err<0)
    goto eof;
  else if ((int)m->in->fmt.ctx->nb_streams<=m->pkt->stream_index) {
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(m->pkt);
#endif // ]
    goto read;
  }

  if (ff_input_progress(m->in,m->pkt)<0)
    goto eof;

  if (m->in->vi==m->pkt->stream_index) {
    // a video packet has just to be remuxed.
    err=ff_muxer_remux_frame(m,m->pkt,m->out->vi);
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(m->pkt);
#endif // ]

    switch (err) {
    case 0:
      break;
    case AVERROR(EINVAL):
      ff_printer_flush(m->in->printer);
      DVWARNING("remuxing video frame: %s (%d)",av_err2str(err),err);
      break;
    default:
      if (err<0) {
        DVMESSAGE("remuxing video frame: %s (%d)",av_err2str(err),err);
        goto e_loop;
      }

      break;
    }

    goto read;
  }
  else if (m->in->ai==m->pkt->stream_index&&!m->in->audio.ctx) {
    // an audio packet has just to be remuxed.
    err=ff_muxer_remux_frame(m,m->pkt,m->out->ai);
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(m->pkt);
#endif // ]

    switch (err) {
    case 0:
      break;
    case AVERROR(EINVAL):
      ff_printer_flush(m->in->printer);
      DVWARNING("remuxing video frame: %s (%d)",av_err2str(err),err);
      break;
    default:
      if (err<0) {
        DVMESSAGE("remuxing video frame: %s (%d)",av_err2str(err),err);
        goto e_loop;
      }

      break;
    }

    goto read;
  }
  else if (m->in->ai!=m->pkt->stream_index) {
    // a packet from a stream we're not interested in has to be skipped.
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(m->pkt);
#endif // ]
    goto read;
  }

  // an audio packet has been read which has to be decoded, re-encoded,
  // and muxed. the packet is unrefed by muxer_send_packet().
  err=ff_muxer_send_packet(m,m->pkt);

  if (FF_MUXER_DECODER_SEND_PACKET==m->state) {
    switch (err) {
    case AVERROR(EAGAIN):
      goto read;
    case AVERROR_EOF:
      goto eof;
    default:
      DVMESSAGE("re-encoding frame: %s (%d)",av_err2str(err),err);
      goto e_loop;
    }
  }
eof:
  /////////////////////////////////////////////////////////////////////////////
  if (m->in->audio.ctx) {
    // we need to flush the decoder/filter/encoder.
    err=ff_muxer_send_packet(m,NULL);

    if (err<0&&FF_MUXER_DECODER_SEND_PACKET<m->state) {
      DVMESSAGE("re-encoding frame: %s (%d:%d)",
          av_err2str(err),err,m->state);
      goto e_loop;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  ff_printer_flush(m->in->printer);

  /////////////////////////////////////////////////////////////////////////////
  return 0;
e_loop:
  return err;
}
