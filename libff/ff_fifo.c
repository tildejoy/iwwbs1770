/*
 * ff_fifo.c
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
// fifo not used. just for reference.
int ff_fifo_create(ff_fifo_t *fifo, const AVCodecParameters *ocodecpar)
{
  int err=-1;

  /////////////////////////////////////////////////////////////////////////////
  fifo->ocodecpar=*ocodecpar;
  fifo->flush=0;

  /////////////////////////////////////////////////////////////////////////////
  fifo->fifo=av_audio_fifo_alloc(ocodecpar->format,ocodecpar->channels,1);

  if (!fifo->fifo) {
    DMESSAGE("allocating audio fifo");
    err=AVERROR(ENOMEM);
    goto e_fifo;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  av_audio_fifo_free(fifo->fifo);
e_fifo:
  return err;
}

void ff_fifo_destroy(ff_fifo_t *fifo)
{
  av_audio_fifo_free(fifo->fifo);
}

///////////////////////////////////////////////////////////////////////////////
int ff_fifo_size(ff_fifo_t *fifo)
{
  return av_audio_fifo_size(fifo->fifo);
}

int ff_fifo_send_frame(ff_fifo_t *fifo, AVFrame *frame)
{
  int err=-1;
  int frame_size=frame->nb_samples;
  int fifo_size=av_audio_fifo_size(fifo->fifo);

  if (!frame) {
    fifo->flush=1;
    return 0;
  }

  // make the fifo as large as it needs to be to hold both,
  // the old and the new samples.
  err=av_audio_fifo_realloc(fifo->fifo,fifo_size+frame_size);

  if (err<0) {
    DMESSAGE("reallocating");
    goto e_realloc;
  }

  // store the new samples in the fifo buffer.
  err=av_audio_fifo_write(fifo->fifo,(void **)frame->data,frame->nb_samples);

  if (err<frame->nb_samples) {
    DMESSAGE("writing");
    err=AVERROR_EXIT;
    goto e_write;
  }

  /////////////////////////////////////////////////////////////////////////////
DVWRITELN("%d",frame->nb_samples);
  return 0;
e_write:
e_realloc:
  return err;
}

int ff_fifo_receive_frame(ff_fifo_t *fifo, AVFrame *frame)
{
  int err=-1;
  int fifo_size=av_audio_fifo_size(fifo->fifo);
  int frame_size=fifo->ocodecpar.frame_size<fifo_size
      ?fifo->ocodecpar.frame_size:fifo_size;;

  /////////////////////////////////////////////////////////////////////////////
  if (fifo->flush&&!fifo_size)
    return AVERROR_EOF;
  else if (!fifo->flush&&fifo_size<fifo->ocodecpar.frame_size)
    return AVERROR(EAGAIN);

  /////////////////////////////////////////////////////////////////////////////
  // set the frame's parameters, especially its size and format.
  // av_frame_get_buffer needs this to allocate memory for the
  // audio samples of the frame.
  // default channel layouts based on the number of channels
  // are assumed for simplicity.
  frame->nb_samples=frame_size;
  frame->channel_layout=fifo->ocodecpar.channel_layout;
  frame->format=fifo->ocodecpar.format;
  frame->sample_rate=fifo->ocodecpar.sample_rate;

  /////////////////////////////////////////////////////////////////////////////
  err=av_frame_get_buffer(frame,0);

  if (err<0) {
    DMESSAGE("getting frame buffer");
    goto e_buffer;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=av_audio_fifo_read(fifo->fifo,(void **)frame->data,frame->nb_samples);

  if (err<frame->nb_samples) {
    DMESSAGE("reading from fifo");
    err=AVERROR_EXIT;
    goto e_read;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_read:
e_buffer:
  return err;
}
