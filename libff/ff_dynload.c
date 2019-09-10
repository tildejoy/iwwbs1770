/*
 * ff_dynload.c
 *
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
#include <ff.h>
#include <pbutil_priv.h>
#if defined (HAVE_FF_DYNLOAD) // [
#if defined (_WIN32) // [
#include <windows.h>
#else // ] [
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if 1 // [
#define DLOPEN_FLAG RTLD_LAZY
#else // ] [
#define DLOPEN_FLAG (RTLD_NOW|RTLD_GLOBAL)
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static struct _ff_avutil {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avutil_version)(void);
  AVFrame *(*av_frame_alloc)(void);
  void (*av_frame_free)(AVFrame **frame);
  int (*av_get_channel_layout_nb_channels)(uint64_t channel_layout);
#if 0 // [
  int64_t (*av_frame_get_best_effort_timestamp)(const AVFrame *frame);
  void (*av_frame_set_best_effort_timestamp)(AVFrame *frame, int64_t val);
  int (*av_log_get_level)(void);
#endif // ]
  void (*av_log_set_level)(int level);
#if 0 // [
  const char *(*av_get_sample_fmt_name)(enum AVSampleFormat sample_fmt);
#if 0 // [
  void (*av_log)(void *avcl, int level, const char *fmt, ...)
      av_printf_format(3, 4);
#else // ] [
  void (*av_vlog)(void *avcl, int level, const char *fmt, va_list vl);
#endif // ]
#endif // ]
  int64_t (*av_rescale_q_rnd)(int64_t a, AVRational bq, AVRational cq,
      enum AVRounding) av_const;
  int64_t (*av_rescale_q)(int64_t a, AVRational bq, AVRational cq) av_const;
#if defined (FF_AV_RESCALE) // [
  int64_t (*av_rescale)(int64_t a, int64_t b, int64_t c);
#endif // ]
#if 0 // [
  void (*av_frame_set_channel_layout)(AVFrame *frame, int64_t val);
  int64_t (*av_frame_get_channel_layout)(const AVFrame *frame);
  void (*av_frame_set_channels)(AVFrame *frame, int val);
#if defined (FF_AV_FRAME_GET_CHANNELS) // [
  int (*av_frame_get_channels)(const AVFrame *frame);
#endif // ]
  void (*av_frame_set_sample_rate)(AVFrame *frame, int val);
  int  (*av_frame_get_sample_rate)(const AVFrame *frame);
  int (*av_samples_alloc)(uint8_t **audio_data, int *linesize,
      int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt,
      int align);
  void (*av_free)(void *ptr);
  void (*av_freep)(void *ptr);
#endif // ]
  int (*av_dict_copy)(AVDictionary **dst, const AVDictionary *src, int flags);
  AVDictionaryEntry *(*av_dict_get)(const AVDictionary *m, const char *key,
      const AVDictionaryEntry *prev, int flags);
  int (*av_dict_set)(AVDictionary **pm, const char *key, const char *value,
      int flags);
  void (*av_dict_free)(AVDictionary **m);
  int (*av_frame_get_buffer)(AVFrame *frame, int align);
#if 0 // [
#if defined (FF_FILTER_CHANNELS) // [
  int (*av_get_channel_layout_channel_index)(uint64_t channel_layout,
      int index);
#endif // ]
#endif // ]
  ////
  void (*av_frame_unref)(AVFrame *frame);
  int (*av_opt_set_int)(void *obj, const char *name, int64_t val,
      int search_flags);
  int (*av_opt_set_sample_fmt)(void *obj, const char *name,
      enum AVSampleFormat fmt, int search_flags);
  int (*av_strerror)(int errnum, char *errbuf, size_t errbuf_size);
#if 0 // [
  AVAudioFifo *(*av_audio_fifo_alloc)(enum AVSampleFormat sample_fmt,
      int channels, int nb_samples);
  void (*av_audio_fifo_free)(AVAudioFifo *af);
  int (*av_audio_fifo_realloc)(AVAudioFifo *af, int nb_samples);
  int (*av_audio_fifo_write)(AVAudioFifo *af, void **data, int nb_samples);
  int (*av_audio_fifo_read)(AVAudioFifo *af, void **data, int nb_samples);
  int (*av_audio_fifo_size)(AVAudioFifo *af);
#endif // ]
  char *(*av_get_sample_fmt_string)(char *buf, int buf_size,
      enum AVSampleFormat sample_fmt);
  void (*av_get_channel_layout_string)(char *buf, int buf_size,
      int nb_channels, uint64_t channel_layout);
  const char *(*av_get_sample_fmt_name)(enum AVSampleFormat sample_fmt);
  unsigned (*av_int_list_length_for_size)(unsigned elsize, const void *list,
      uint64_t term);
  int (*av_opt_set_bin)(void *obj, const char *name, const uint8_t *val,
      int size, int search_flags);
  char *(*av_strdup)(const char *s);
  void (*av_vlog)(void *avcl, int level, const char *fmt, va_list vl);
  int64_t (*av_get_default_channel_layout)(int nb_channels);
#if 0 // [
  AVBufferRef *(*av_buffer_ref)(AVBufferRef *buf);
#endif // ]
} avutil;

static int avutil_load_sym(void *p, const char *sym);

// [
unsigned avutil_version(void)
{
  if (avutil_load_sym(&avutil.avutil_version,__func__)<0)
    return 0u;

  return avutil.avutil_version();
}

AVFrame *av_frame_alloc(void)
{
  if (avutil_load_sym(&avutil.av_frame_alloc,__func__)<0)
    return NULL;

  return avutil.av_frame_alloc();
}

void av_frame_free(AVFrame **frame)
{
  if (avutil_load_sym(&avutil.av_frame_free,__func__)<0)
    return;

  avutil.av_frame_free(frame);
}

int av_get_channel_layout_nb_channels(uint64_t channel_layout)
{
  if (avutil_load_sym(&avutil.av_get_channel_layout_nb_channels,__func__)<0)
    return -1;

  return avutil.av_get_channel_layout_nb_channels(channel_layout);
}

#if 0 // [
int64_t av_frame_get_best_effort_timestamp(const AVFrame *frame)
{
  if (avutil_load_sym(&avutil.av_frame_get_best_effort_timestamp,__func__)<0)
    return -1;

  return avutil.av_frame_get_best_effort_timestamp(frame);
}

void av_frame_set_best_effort_timestamp(AVFrame *frame, int64_t val)
{
  if (avutil_load_sym(&avutil.av_frame_set_best_effort_timestamp,__func__)<0)
    return;

  avutil.av_frame_set_best_effort_timestamp(frame,val);
}

int av_log_get_level(void)
{
  if (avutil_load_sym(&avutil.av_log_get_level,__func__)<0)
    return -1;

  return avutil.av_log_get_level();
}
#endif // ]

void av_log_set_level(int level)
{
  if (avutil_load_sym(&avutil.av_log_set_level,__func__)<0)
    return;

  avutil.av_log_set_level(level);
}

#if 0 // [
const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt)
{
  if (avutil_load_sym(&avutil.av_get_sample_fmt_name,__func__)<0)
    return NULL;

  return avutil.av_get_sample_fmt_name(sample_fmt);
}

void av_log(void *avcl, int level, const char *fmt, ...)
{
  va_list ap;

  if (avutil_load_sym(&avutil.av_vlog,__func__)<0)
    return;

  va_start(ap,fmt);
  avutil.av_vlog(avcl,level,fmt,ap);
  va_end(ap);
}
#endif // ]

int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
    enum AVRounding rnd)
{
  if (avutil_load_sym(&avutil.av_rescale_q_rnd,__func__)<0)
    return -1;

  return avutil.av_rescale_q_rnd(a,bq,cq,rnd);
}

int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
{
  if (avutil_load_sym(&avutil.av_rescale_q,__func__)<0)
    return -1;

  return avutil.av_rescale_q(a,bq,cq);
}

#if defined (FF_AV_RESCALE) // [
int64_t av_rescale(int64_t a, int64_t b, int64_t c)
{
  if (avutil_load_sym(&avutil.av_rescale,__func__)<0)
    return -1ll;

  return avutil.av_rescale(a,b,c);
}
#endif // ]

#if 0 // [
void av_frame_set_channel_layout(AVFrame *frame, int64_t val)
{
  if (avutil_load_sym(&avutil.av_frame_set_channel_layout,__func__)<0)
    return;

  avutil.av_frame_set_channel_layout(frame,val);
}

int64_t av_frame_get_channel_layout(const AVFrame *frame)
{
  if (avutil_load_sym(&avutil.av_frame_get_channel_layout,__func__)<0)
    return -1;

  return avutil.av_frame_get_channel_layout(frame);
}

void av_frame_set_channels(AVFrame *frame, int val)
{
  if (avutil_load_sym(&avutil.av_frame_set_channels,__func__)<0)
    return;

  avutil.av_frame_set_channels(frame,val);
}

#if defined (FF_AV_FRAME_GET_CHANNELS) // [
int av_frame_get_channels(const AVFrame *frame)
{
  if (avutil_load_sym(&avutil.av_frame_get_channels,__func__)<0)
    return -1;

  return avutil.av_frame_get_channels(frame);
}
#endif // ]

void av_frame_set_sample_rate(AVFrame *frame, int val)
{
  if (avutil_load_sym(&avutil.av_frame_set_sample_rate,__func__)<0)
    return;

  avutil.av_frame_set_sample_rate(frame,val);
}

int  av_frame_get_sample_rate(const AVFrame *frame)
{
  if (avutil_load_sym(&avutil.av_frame_get_sample_rate,__func__)<0)
    return -1;

  return avutil.av_frame_get_sample_rate(frame);
}

int av_samples_alloc(uint8_t **audio_data, int *linesize,
    int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt,
    int align)
{
  if (avutil_load_sym(&avutil.av_samples_alloc,__func__)<0)
    return -1;

  return avutil.av_samples_alloc(audio_data,linesize,nb_channels,
      nb_samples,sample_fmt,align);
}

void av_free(void *ptr)
{
  if (avutil_load_sym(&avutil.av_free,__func__)<0)
    return;

  avutil.av_free(ptr);
}

void av_freep(void *ptr)
{
  if (avutil_load_sym(&avutil.av_freep,__func__)<0)
    return;

  avutil.av_freep(ptr);
}
#endif // ]

int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int	flags)
{
  if (avutil_load_sym(&avutil.av_dict_copy,__func__)<0)
    return -1;

  return avutil.av_dict_copy(dst,src,flags);
}

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key,
    const AVDictionaryEntry *prev, int flags)
{
  if (avutil_load_sym(&avutil.av_dict_get,__func__)<0)
    return NULL;

  return avutil.av_dict_get(m,key,prev,flags);
}

int av_dict_set(AVDictionary **pm, const char *key, const char *value,
    int flags)
{
  if (avutil_load_sym(&avutil.av_dict_set,__func__)<0)
    return -1;

  return avutil.av_dict_set(pm,key,value,flags);
}

void av_dict_free(AVDictionary **m)
{
  if (avutil_load_sym(&avutil.av_dict_free,__func__)<0)
    return;

  avutil.av_dict_free(m);
}

int av_frame_get_buffer(AVFrame *frame, int align)
{
  if (avutil_load_sym(&avutil.av_frame_get_buffer,__func__)<0)
    return -1;

  return avutil.av_frame_get_buffer(frame,align);
}

#if 0 // [
#if defined (FF_FILTER_CHANNELS) // [
int av_get_channel_layout_channel_index(uint64_t channel_layout,
    int index)
{
  if (avutil_load_sym(&avutil.av_get_channel_layout_channel_index,__func__)<0)
    return -1;

  return avutil.av_get_channel_layout_channel_index(channel_layout,index);
}
#endif // ]
#endif // ]

////
void av_frame_unref(AVFrame *frame)
{
  if (avutil_load_sym(&avutil.av_frame_unref,__func__)<0)
    return;

  avutil.av_frame_unref(frame);
}

int av_opt_set_int(void *obj, const char *name, int64_t val, int search_flags)
{
  if (avutil_load_sym(&avutil.av_opt_set_int,__func__)<0)
    return -1;

  return avutil.av_opt_set_int(obj,name,val,search_flags);
}

int av_opt_set_sample_fmt(void *obj, const char *name, enum AVSampleFormat fmt,
    int search_flags)
{
  if (avutil_load_sym(&avutil.av_opt_set_sample_fmt,__func__)<0)
    return -1;

  return avutil.av_opt_set_sample_fmt(obj,name,fmt,search_flags);
}

int av_strerror(int errnum, char *errbuf, size_t errbuf_size)
{
  if (avutil_load_sym(&avutil.av_strerror,__func__)<0)
    return -1;

  return avutil.av_strerror(errnum,errbuf,errbuf_size);
}

#if 0 // [
AVAudioFifo *av_audio_fifo_alloc(enum AVSampleFormat sample_fmt, int channels,
    int nb_samples)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_alloc,__func__)<0)
    return NULL;

  return avutil.av_audio_fifo_alloc(sample_fmt,channels,nb_samples);
}

void av_audio_fifo_free(AVAudioFifo *af)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_free,__func__)<0)
    return;

  avutil.av_audio_fifo_free(af);
}

int av_audio_fifo_realloc(AVAudioFifo *af, int nb_samples)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_realloc,__func__)<0)
    return -1;

  return avutil.av_audio_fifo_realloc(af,nb_samples);
}

int av_audio_fifo_write(AVAudioFifo *af, void **data, int nb_samples)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_write,__func__)<0)
    return -1;

  return avutil.av_audio_fifo_write(af,data,nb_samples);
}

int av_audio_fifo_read(AVAudioFifo *af, void **data, int nb_samples)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_read,__func__)<0)
    return -1;

  return avutil.av_audio_fifo_read(af,data,nb_samples);
}

int av_audio_fifo_size(AVAudioFifo *af)
{
  if (avutil_load_sym(&avutil.av_audio_fifo_size,__func__)<0)
    return -1;

  return avutil.av_audio_fifo_size(af);
}
#endif // ]

char *av_get_sample_fmt_string(char *buf, int buf_size,
    enum AVSampleFormat sample_fmt)
{
  if (avutil_load_sym(&avutil.av_get_sample_fmt_string,__func__)<0)
    return NULL;

  return avutil.av_get_sample_fmt_string(buf,buf_size,sample_fmt);
}

void av_get_channel_layout_string(char *buf, int buf_size, int nb_channels,
    uint64_t channel_layout)
{
  if (avutil_load_sym(&avutil.av_get_channel_layout_string,__func__)<0)
    return;

  avutil.av_get_channel_layout_string(buf,buf_size,nb_channels,channel_layout);
}

const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt)
{
  if (avutil_load_sym(&avutil.av_get_sample_fmt_name,__func__)<0)
    return NULL;

  return avutil.av_get_sample_fmt_name(sample_fmt);
}

unsigned av_int_list_length_for_size(unsigned elsize, const void *list,
    uint64_t term)
{
  if (avutil_load_sym(&avutil.av_int_list_length_for_size,__func__)<0)
    return 0u;

  return avutil.av_int_list_length_for_size(elsize,list,term);
}

int av_opt_set_bin(void *obj, const char *name, const uint8_t *val, int size,
    int search_flags)
{
  if (avutil_load_sym(&avutil.av_opt_set_bin,__func__)<0)
    return -1;

  return avutil.av_opt_set_bin(obj,name,val,size,search_flags);
}

char *av_strdup(const char *s)
{
  if (avutil_load_sym(&avutil.av_strdup,__func__)<0)
    return NULL;

  return avutil.av_strdup(s);
}

void av_log(void *avcl, int level, const char *fmt, ...)
{
  va_list ap;

  if (avutil_load_sym(&avutil.av_vlog,__func__)<0)
    return;

  va_start(ap,fmt);
  avutil.av_vlog(avcl,level,fmt,ap);
  va_end(ap);
}

int64_t av_get_default_channel_layout(int nb_channels)
{
  if (avutil_load_sym(&avutil.av_get_default_channel_layout,__func__)<0)
    return 0ll;

  return avutil.av_get_default_channel_layout(nb_channels);
}

#if 0 // [
AVBufferRef *av_buffer_ref(AVBufferRef *buf)
{
  if (avutil_load_sym(&avutil.av_buffer_ref,__func__)<0)
    return NULL;

  return avutil.av_buffer_ref(buf);
}
#endif // ]
// ]

///////////////////////////////////////////////////////////////////////////////
static struct _ff_avcodec {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avcodec_version)(void);
#if (LIBAVCODEC_VERSION_MAJOR<58) // [
  void (*avcodec_register_all)(void);
#endif // ]
  AVCodec *(*avcodec_find_decoder)(enum AVCodecID id);
  AVCodec *(*avcodec_find_decoder_by_name)(const char *name);
  AVCodec *(*avcodec_find_encoder)(enum AVCodecID id);
  int (*avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec,
      AVDictionary **options);
#if 0 // [
  void (*av_init_packet)(AVPacket *pkt);
#endif // ]
  int (*avcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt);
  int (*avcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame);
  int (*avcodec_send_frame)(AVCodecContext *avctx, const AVFrame *frame);
  int (*avcodec_receive_packet)(AVCodecContext *avctx, AVPacket *avpkt);
#if 0 // [
  int (*avcodec_decode_audio4)(AVCodecContext *avctx, AVFrame *frame,
      int *got_frame_ptr, const AVPacket *avpkt);
  int (*avcodec_encode_audio2)(AVCodecContext *avctx, AVPacket *avpkt,
      const AVFrame *frame, int *got_packet_ptr);
  int (*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture,
      int *got_picture_ptr, const AVPacket *avpkt);
#endif // ]
  void (*av_packet_unref)(AVPacket *pkt);
  int (*avcodec_close)(AVCodecContext *avctx);
#if 0 // [
  int (*avcodec_copy_context)(AVCodecContext *dest, const AVCodecContext *src);
#endif // ]
  void (*av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src,
      AVRational tb_dst);
  ////
  AVCodecContext *(*avcodec_alloc_context3)(const AVCodec *codec);
  void (*avcodec_free_context)(AVCodecContext **avctx);
  AVPacket *(*av_packet_alloc)(void);
  void (*av_packet_free)(AVPacket **pkt);
  int (*avcodec_parameters_copy)(AVCodecParameters *dst,
      const AVCodecParameters *src);
  int (*avcodec_parameters_to_context)(AVCodecContext *codec,
      const AVCodecParameters *par);
  int (*avcodec_parameters_from_context)(AVCodecParameters *par,
      const AVCodecContext *codec);
  const char *(*avcodec_get_name)(enum AVCodecID id);
  void (*avcodec_flush_buffers)(AVCodecContext *avctx);
  AVCodec *(*avcodec_find_encoder_by_name)(const char *name);
  const AVCodec *(*av_codec_iterate)(void **opaque);
#if 0 // [
  void (*av_init_packet)(AVPacket *pkt);
  int (*av_packet_copy_props)(AVPacket *dst, const AVPacket *src);
#endif // ]
} avcodec;

static int avcodec_load_sym(void *p, const char *sym);

// [
unsigned avcodec_version(void)
{
#if 0 // [
  if (avcodec_load_sym(&avcodec.avcodec_version,__func__)<0)
    return 0u;

  return avcodec.avcodec_version();
#else // ] [
  unsigned version=0u;

  if (avcodec_load_sym(&avcodec.avcodec_version,__func__)<0)
    goto exit;

  version=avcodec.avcodec_version();
exit:
  return version;
#endif // ]
}

#if (LIBAVCODEC_VERSION_MAJOR<58) // [
void avcodec_register_all(void)
{
  if (avcodec_load_sym(&avcodec.avcodec_register_all,__func__)<0)
    return;

  avcodec.avcodec_register_all();
}
#endif // ]

AVCodec *avcodec_find_decoder(enum AVCodecID id)
{
  if (avcodec_load_sym(&avcodec.avcodec_find_decoder,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_decoder(id);
}

AVCodec *avcodec_find_decoder_by_name(const char *name)
{
  if (avcodec_load_sym(&avcodec.avcodec_find_decoder_by_name,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_decoder_by_name(name);
}

AVCodec *avcodec_find_encoder(enum AVCodecID id)
{
  if (avcodec_load_sym(&avcodec.avcodec_find_encoder,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_encoder(id);
}

int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec,
    AVDictionary **options)
{
  if (avcodec_load_sym(&avcodec.avcodec_open2,__func__)<0)
    return -1;

  return avcodec.avcodec_open2(avctx,codec,options);
}

#if 0 // [
void av_init_packet(AVPacket *pkt)
{
  if (avcodec_load_sym(&avcodec.av_init_packet,__func__)<0)
    return;

  avcodec.av_init_packet(pkt);
}
#endif // ]

int avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt)
{
  if (avcodec_load_sym(&avcodec.avcodec_send_packet,__func__)<0)
    return -1;

  return avcodec.avcodec_send_packet(avctx,avpkt);
}

int avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
  if (avcodec_load_sym(&avcodec.avcodec_receive_frame,__func__)<0)
    return -1;

  return avcodec.avcodec_receive_frame(avctx,frame);
}

int avcodec_send_frame(AVCodecContext *avctx, const AVFrame *frame)
{
  if (avcodec_load_sym(&avcodec.avcodec_send_frame,__func__)<0)
    return -1;

  return avcodec.avcodec_send_frame(avctx,frame);
}

int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt)
{
  if (avcodec_load_sym(&avcodec.avcodec_receive_packet,__func__)<0)
    return -1;

  return avcodec.avcodec_receive_packet(avctx,avpkt);
}

#if 0 // [
int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
    int *got_frame_ptr, const AVPacket *avpkt)
{
  if (avcodec_load_sym(&avcodec.avcodec_decode_audio4,__func__)<0)
    return -1;

  return avcodec.avcodec_decode_audio4(avctx,frame,got_frame_ptr,avpkt);
}

int avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt,
    const AVFrame *frame, int *got_packet_ptr)
{
  if (avcodec_load_sym(&avcodec.avcodec_encode_audio2,__func__)<0)
    return -1;

  return avcodec.avcodec_encode_audio2(avctx,avpkt,frame,got_packet_ptr);
}

int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
    int *got_picture_ptr,const AVPacket *avpkt)
{
  if (avcodec_load_sym(&avcodec.avcodec_decode_video2,__func__)<0)
    return -1;

  return avcodec.avcodec_decode_video2(avctx,picture,got_picture_ptr,avpkt);
}
#endif // ]

void av_packet_unref(AVPacket *pkt)
{
  if (avcodec_load_sym(&avcodec.av_packet_unref,__func__)<0)
    return;

  avcodec.av_packet_unref(pkt);
}

int avcodec_close(AVCodecContext *avctx)
{
  if (avcodec_load_sym(&avcodec.avcodec_close,__func__)<0)
    return -1;

  return avcodec.avcodec_close(avctx);
}

#if 0 // [
int avcodec_copy_context(AVCodecContext *dest, const AVCodecContext *src)
{
  if (avcodec_load_sym(&avcodec.avcodec_copy_context,__func__)<0)
    return -1;

  return avcodec.avcodec_copy_context(dest,src);
}
#endif // ]

void av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src,
    AVRational tb_dst)
{
  if (avcodec_load_sym(&avcodec.av_packet_rescale_ts,__func__)<0)
    return;

  avcodec.av_packet_rescale_ts(pkt,tb_src,tb_dst);
}

////
AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)
{
  if (avcodec_load_sym(&avcodec.avcodec_alloc_context3,__func__)<0)
    return NULL;

  return avcodec.avcodec_alloc_context3(codec);
}

void avcodec_free_context(AVCodecContext **avctx)
{
  if (avcodec_load_sym(&avcodec.avcodec_free_context,__func__)<0)
    return;

  avcodec.avcodec_free_context(avctx);
}

AVPacket *av_packet_alloc(void)
{
  if (avcodec_load_sym(&avcodec.av_packet_alloc,__func__)<0)
    return NULL;

  return avcodec.av_packet_alloc();
}

void av_packet_free(AVPacket **pkt)
{
  if (avcodec_load_sym(&avcodec.av_packet_free,__func__)<0)
    return;

  avcodec.av_packet_free(pkt);
}

int avcodec_parameters_copy(AVCodecParameters *dst,
    const AVCodecParameters *src)
{
  if (avcodec_load_sym(&avcodec.avcodec_parameters_copy,__func__)<0)
    return -1;

  return avcodec.avcodec_parameters_copy(dst,src);
}

int avcodec_parameters_to_context(AVCodecContext *codec,
    const AVCodecParameters *par)
{
  if (avcodec_load_sym(&avcodec.avcodec_parameters_to_context,__func__)<0)
    return -1;

  return avcodec.avcodec_parameters_to_context(codec,par);
}

int avcodec_parameters_from_context(AVCodecParameters *par,
    const AVCodecContext *codec)
{
  if (avcodec_load_sym(&avcodec.avcodec_parameters_from_context,__func__)<0)
    return -1;

  return avcodec.avcodec_parameters_from_context(par,codec);
}

const char *avcodec_get_name(enum AVCodecID id)
{
  if (avcodec_load_sym(&avcodec.avcodec_get_name,__func__)<0)
    return NULL;

  return avcodec.avcodec_get_name(id);
}

void avcodec_flush_buffers(AVCodecContext *avctx)
{
  if (avcodec_load_sym(&avcodec.avcodec_flush_buffers,__func__)<0)
    return;

  avcodec.avcodec_flush_buffers(avctx);
}

AVCodec *avcodec_find_encoder_by_name(const char *name)
{
  if (avcodec_load_sym(&avcodec.avcodec_find_encoder_by_name,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_encoder_by_name(name);
}

const AVCodec *av_codec_iterate(void **opaque)
{
  if (avcodec_load_sym(&avcodec.av_codec_iterate,__func__)<0)
    return NULL;

  return avcodec.av_codec_iterate(opaque);
}

#if 0 // [
void av_init_packet(AVPacket *pkt)
{
  if (avcodec_load_sym(&avcodec.av_init_packet,__func__)<0)
    return;

  avcodec.av_init_packet(pkt);
}

int av_packet_copy_props(AVPacket *dst, const AVPacket *src)
{
  if (avcodec_load_sym(&avcodec.av_packet_copy_props,__func__)<0)
    return -1;

  return avcodec.av_packet_copy_props(dst,src);
}
#endif // ]
// ]

///////////////////////////////////////////////////////////////////////////////
static struct _ff_avformat {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avformat_version)(void);
#if (LIBAVFORMAT_VERSION_MAJOR<58) // [
  void (*av_register_all)(void);
#endif // ]
  int (*avformat_open_input)(AVFormatContext **ps, const char *filename,
      AVInputFormat *fmt, AVDictionary **options);
  int (*avformat_find_stream_info)(AVFormatContext *ic,
      AVDictionary **options);
  int (*av_read_frame)(AVFormatContext *s, AVPacket *pkt);
  void (*avformat_close_input)(AVFormatContext **s);
  int (*avformat_alloc_output_context2)(AVFormatContext **ctx,
      AVOutputFormat *oformat, const char *format_name,
      const char *filename);
  void (*avformat_free_context)(AVFormatContext *s);
  AVStream *(*avformat_new_stream)(AVFormatContext *s, const AVCodec *c);
  int (*avio_open)(AVIOContext **s, const char *url, int flags);
  int (*avio_closep)(AVIOContext **s);
  int (*avformat_write_header)(AVFormatContext *s, AVDictionary **options);
  int (*av_interleaved_write_frame)(AVFormatContext *s, AVPacket *pkt);
  int (*av_write_trailer)(AVFormatContext *s);
#if 0 // [
  int (*av_find_default_stream_index)(AVFormatContext *s);
#endif // ]
  void (*av_dump_format)(AVFormatContext *ic, int index, const char *url,
      int is_output);
  int (*avformat_seek_file)(AVFormatContext *s, int stream_index,
      int64_t min_ts, int64_t ts, int64_t max_ts, int flags);
  int (*av_seek_frame)(AVFormatContext *s, int stream_index,
      int64_t timestamp, int flags);
  int (*avformat_flush)(AVFormatContext *s);
  const AVOutputFormat *(*av_muxer_iterate)(void **opaque);
  int (*avformat_query_codec)(const AVOutputFormat *ofmt,
      enum AVCodecID codec_id, int std_compliance);
} avformat;

static int avformat_load_sym(void *p, const char *sym);

// [
unsigned avformat_version(void)
{
  if (avformat_load_sym(&avformat.avformat_version,__func__)<0)
    return 0u;

  return avformat.avformat_version();
}

#if (LIBAVFORMAT_VERSION_MAJOR<58) // [
void av_register_all(void)
{
  if (avformat_load_sym(&avformat.av_register_all,__func__)<0)
    return;

  avformat.av_register_all();
}
#endif // ]

int avformat_open_input(AVFormatContext **ps, const char *filename,
    AVInputFormat *fmt, AVDictionary **options)
{
  if (avformat_load_sym(&avformat.avformat_open_input,__func__)<0)
    return -1;

  return avformat.avformat_open_input(ps,filename,fmt,options);
}

int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
{
  if (avformat_load_sym(&avformat.avformat_find_stream_info,__func__)<0)
    return -1;

  return avformat.avformat_find_stream_info(ic,options);
}

int av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
  if (avformat_load_sym(&avformat.av_read_frame,__func__)<0)
    return -1;

  return avformat.av_read_frame(s,pkt);
}

void avformat_close_input(AVFormatContext **s)
{
  if (avformat_load_sym(&avformat.avformat_close_input,__func__)<0)
    return;

  avformat.avformat_close_input(s);
}

int avformat_alloc_output_context2(AVFormatContext **ctx,
    AVOutputFormat *oformat, const char *format_name,
    const char *filename)
{
  if (avformat_load_sym(&avformat.avformat_alloc_output_context2,__func__)<0)
    return -1;

  return avformat.avformat_alloc_output_context2(ctx,oformat,format_name,
      filename);
}

void avformat_free_context(AVFormatContext *s)
{
  if (avformat_load_sym(&avformat.avformat_free_context,__func__)<0)
    return;

  avformat.avformat_free_context(s);
}

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c)
{
  if (avformat_load_sym(&avformat.avformat_new_stream,__func__)<0)
    return NULL;

  return avformat.avformat_new_stream(s,c);
}

int avio_open(AVIOContext **s, const char *url, int flags)
{
  if (avformat_load_sym(&avformat.avio_open,__func__)<0)
    return -1;

  return avformat.avio_open(s,url,flags);
}

int avio_closep(AVIOContext **s)
{
  if (avformat_load_sym(&avformat.avio_closep,__func__)<0)
    return -1;

  return avformat.avio_closep(s);
}

int avformat_write_header(AVFormatContext *s, AVDictionary **options)
{
  if (avformat_load_sym(&avformat.avformat_write_header,__func__)<0)
    return -1;

  return avformat.avformat_write_header(s,options);
}

int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt)
{
  if (avformat_load_sym(&avformat.av_interleaved_write_frame,__func__)<0)
    return -1;

  return avformat.av_interleaved_write_frame(s,pkt);
}

int av_write_trailer(AVFormatContext *s)
{
  if (avformat_load_sym(&avformat.av_write_trailer,__func__)<0)
    return -1;

  return avformat.av_write_trailer(s);
}

#if 0 // [
int av_find_default_stream_index(AVFormatContext *s)
{
  if (avformat_load_sym(&avformat.av_find_default_stream_index,__func__)<0)
    return -1;

  return avformat.av_find_default_stream_index(s);
}
#endif // ]

void av_dump_format(AVFormatContext *ic, int index, const char *url,
    int is_output)
{
  if (avformat_load_sym(&avformat.av_dump_format,__func__)<0)
    return;

  avformat.av_dump_format(ic,index,url,is_output);
}

int avformat_seek_file(AVFormatContext *s, int stream_index,
    int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
{
  if (avformat_load_sym(&avformat.avformat_seek_file,__func__)<0)
    return -1;

  return avformat.avformat_seek_file(s,stream_index,min_ts,ts,max_ts,flags);
}

int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,
    int flags)
{
  if (avformat_load_sym(&avformat.av_seek_frame,__func__)<0)
    return -1;

  return avformat.av_seek_frame(s,stream_index,timestamp,flags);
}

int avformat_flush(AVFormatContext *s)
{
  if (avformat_load_sym(&avformat.avformat_flush,__func__)<0)
    return -1;

  return avformat.avformat_flush(s);
}

const AVOutputFormat *av_muxer_iterate(void **opaque)
{
  if (avformat_load_sym(&avformat.av_muxer_iterate,__func__)<0)
    return NULL;

  return avformat.av_muxer_iterate(opaque);
}

int avformat_query_codec(const AVOutputFormat *ofmt, enum AVCodecID codec_id,
    int std_compliance)
{
  if (avformat_load_sym(&avformat.avformat_query_codec,__func__)<0)
    return 0;

  return avformat.avformat_query_codec(ofmt,codec_id,std_compliance);
}
// ]

///////////////////////////////////////////////////////////////////////////////
static struct _ff_swresample {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*swresample_version)(void);
  ////
  struct SwrContext *(*swr_alloc)(void);
  void (*swr_free)(struct SwrContext **s);
  int (*swr_init)(struct SwrContext *s);
  void (*swr_close)(struct SwrContext *s);
  struct SwrContext *(*swr_alloc_set_opts)(struct SwrContext *s,
      int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt,
      int out_sample_rate, int64_t in_ch_layout,
      enum AVSampleFormat in_sample_fmt, int in_sample_rate,
      int log_offset, void *log_ctx);
  int (*swr_convert_frame)(SwrContext *swr, AVFrame *output,
      const AVFrame *input);
} swresample;

static int swresample_load_sym(void *p, const char *sym);

unsigned swresample_version(void)
{
  if (swresample_load_sym(&swresample.swresample_version,__func__)<0)
    return 0u;

  return swresample.swresample_version();
}

////
struct SwrContext *swr_alloc(void)
{
  if (swresample_load_sym(&swresample.swr_alloc,__func__)<0)
    return NULL;

  return swresample.swr_alloc();
}

void swr_free(struct SwrContext **s)
{
  if (swresample_load_sym(&swresample.swr_free,__func__)<0)
    return;

  swresample.swr_free(s);
}

int swr_init(struct SwrContext *s)
{
  if (swresample_load_sym(&swresample.swr_init,__func__)<0)
    return -1;

  return swresample.swr_init(s);
}

void swr_close(struct SwrContext *s)
{
  if (swresample_load_sym(&swresample.swr_close,__func__)<0)
    return;

  swresample.swr_close(s);
}

struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
    int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt,
    int out_sample_rate, int64_t in_ch_layout,
    enum AVSampleFormat in_sample_fmt, int in_sample_rate,
    int log_offset, void *log_ctx)
{
  if (swresample_load_sym(&swresample.swr_alloc_set_opts,__func__)<0)
    return NULL;

  return swresample.swr_alloc_set_opts(s,out_ch_layout,out_sample_fmt,
      out_sample_rate,in_ch_layout,in_sample_fmt,in_sample_rate,log_offset,
      log_ctx);
}

int swr_convert_frame(SwrContext *swr, AVFrame *output, const AVFrame *input)
{
  if (swresample_load_sym(&swresample.swr_convert_frame,__func__)<0)
    return -1;

  return swresample.swr_convert_frame(swr,output,input);
}

///////////////////////////////////////////////////////////////////////////////
static struct _ff_swscale {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*swscale_version)(void);
} swscale;

static int swscale_load_sym(void *p, const char *sym);

unsigned swscale_version(void)
{
  if (swscale_load_sym(&swscale.swscale_version,__func__)<0)
    return 0u;

  return swscale.swscale_version();
}

///////////////////////////////////////////////////////////////////////////////
static struct _ff_postproc {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*postproc_version)(void);
} postproc;

static int postproc_load_sym(void *p, const char *sym) FFUNUSED;

unsigned postproc_version(void)
{
  if (postproc_load_sym(&postproc.postproc_version,__func__)<0)
    return 0u;

  return postproc.postproc_version();
}

///////////////////////////////////////////////////////////////////////////////
static struct _ff_avfilter {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avfilter_version)(void);
#if (LIBAVFILTER_VERSION_MAJOR<7) // [
  void (*avfilter_register_all)(void);
#endif // ]
  AVFilterGraph *(*avfilter_graph_alloc)(void);
  const AVFilter *(*avfilter_get_by_name)(const char *name);
  int (*avfilter_graph_create_filter)(AVFilterContext **filt_ctx,
      const AVFilter *filt, const char *name, const char *args,
      void *opaque, AVFilterGraph *graph_ctx);
  int (*avfilter_graph_parse_ptr)(AVFilterGraph *graph, const char *filters,
      AVFilterInOut **inputs, AVFilterInOut **outputs, void *log_ctx);
  AVFilterInOut *(*avfilter_inout_alloc)(void);
  int (*avfilter_graph_config)(AVFilterGraph *graphctx, void *log_ctx);
  void (*avfilter_inout_free)(AVFilterInOut **inout);
  void (*avfilter_graph_free)(AVFilterGraph **graph);
  int (*av_buffersrc_add_frame)(AVFilterContext *ctx, AVFrame *frame);
  int (*av_buffersink_get_frame)(AVFilterContext *ctx, AVFrame *frame);
  void (*av_buffersink_set_frame_size)(AVFilterContext *ctx,
      unsigned frame_size);
} avfilter;

static int avfilter_load_sym(void *p, const char *sym);

// [
unsigned avfilter_version(void)
{
  if (avfilter_load_sym(&avfilter.avfilter_version,__func__)<0)
    return 0u;

  return avfilter.avfilter_version();
}

#if (LIBAVFILTER_VERSION_MAJOR<7) // [
void avfilter_register_all(void)
{
  if (avfilter_load_sym(&avfilter.avfilter_register_all,__func__)<0)
    return;

  avfilter.avfilter_register_all();
}
#endif // ]

AVFilterGraph *avfilter_graph_alloc(void)
{
  if (avfilter_load_sym(&avfilter.avfilter_graph_alloc,__func__)<0)
    return NULL;

  return avfilter.avfilter_graph_alloc();
}

const AVFilter *avfilter_get_by_name(const char *name)
{
  if (avfilter_load_sym(&avfilter.avfilter_get_by_name,__func__)<0)
    return NULL;

  return avfilter.avfilter_get_by_name(name);
}

int avfilter_graph_create_filter(AVFilterContext **filt_ctx,
    const AVFilter *filt, const char *name, const char *args,
    void *opaque, AVFilterGraph *graph_ctx)
{
  if (avfilter_load_sym(&avfilter.avfilter_graph_create_filter,__func__)<0)
    return -1;

  return avfilter.avfilter_graph_create_filter(filt_ctx,filt,name,args,
      opaque,graph_ctx);
}

int avfilter_graph_parse_ptr(AVFilterGraph *graph, const char *filters,
    AVFilterInOut **inputs, AVFilterInOut **outputs, void *log_ctx)
{
  if (avfilter_load_sym(&avfilter.avfilter_graph_parse_ptr,__func__)<0)
    return -1;

  return avfilter.avfilter_graph_parse_ptr(graph,filters,inputs,outputs,
      log_ctx);
}

AVFilterInOut *avfilter_inout_alloc(void)
{
  if (avfilter_load_sym(&avfilter.avfilter_inout_alloc,__func__)<0)
    return NULL;

  return avfilter.avfilter_inout_alloc();
}

int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx)
{
  if (avfilter_load_sym(&avfilter.avfilter_graph_config,__func__)<0)
    return -1;

  return avfilter.avfilter_graph_config(graphctx,log_ctx);
}

void avfilter_inout_free(AVFilterInOut **inout)
{
  if (avfilter_load_sym(&avfilter.avfilter_inout_free,__func__)<0)
    return;

  avfilter.avfilter_inout_free(inout);
}

void avfilter_graph_free(AVFilterGraph **graph)
{
  if (avfilter_load_sym(&avfilter.avfilter_graph_free,__func__)<0)
    return;

  avfilter.avfilter_graph_free(graph);
}

int av_buffersrc_add_frame(AVFilterContext *ctx, AVFrame *frame)
{
  if (avfilter_load_sym(&avfilter.av_buffersrc_add_frame,__func__)<0)
    return -1;

  return avfilter.av_buffersrc_add_frame(ctx,frame);
}

int av_buffersink_get_frame(AVFilterContext *ctx, AVFrame *frame)
{
  if (avfilter_load_sym(&avfilter.av_buffersink_get_frame,__func__)<0)
    return -1;

  return avfilter.av_buffersink_get_frame(ctx,frame);
}


void av_buffersink_set_frame_size(AVFilterContext *ctx, unsigned frame_size)
{
  if (avfilter_load_sym(&avfilter.av_buffersink_set_frame_size,__func__)<0)
    return;

  avfilter.av_buffersink_set_frame_size(ctx,frame_size);
}
// ]

///////////////////////////////////////////////////////////////////////////////
static ffchar_t path[PATH_MAX],*pp=path;

#if defined (_WIN32) // [
static int load(HMODULE hLib, const char *sym, void *p)
#else // ] [
static int load(void *hLib, const char *sym, void *p)
#endif // ]
{
  void **fp=p;

  if (!*fp) {
    *fp=FFDLSYM(hLib,sym);

    if (!*fp) {
      FFVMESSAGE("loading %s",sym);
      return -1;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
static int avutil_load(void);
static int swresample_load(void);
static int avcodec_load(void);
static int avformat_load(void);
static int swscale_load(void);
static int postproc_load(void);
static int avfilter_load(void);

///////////////////////////////////////////////////////////////////////////////
static int avutil_load(void)
{
  static const ffchar_t AVUTIL[]=FF_AVUTIL;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVUTIL=(sizeof AVUTIL)/(sizeof AVUTIL[0]),
  };

  if (!avutil.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVUTIL)) {
      FFVMESSAGE("loading %s",AVUTIL);
      return -1;
   }

    FFSTRCPY(pp,AVUTIL);
    avutil.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!avutil.hLib&&!(avutil.hLib=FFDLOPEN(AVUTIL,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int avcodec_load(void)
{
  static const ffchar_t AVCODEC[]=FF_AVCODEC;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVCODEC=(sizeof AVCODEC)/(sizeof AVCODEC[0]),
  };

  if (avutil_load()<0)
    return -1;
  else if (swresample_load()<0)
    return -1;
  else if (!avcodec.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVCODEC)) {
      FFVMESSAGE("loading %s",AVCODEC);
      return -1;
    }

    FFSTRCPY(pp,AVCODEC);
    avcodec.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!avcodec.hLib&&!(avcodec.hLib=FFDLOPEN(AVCODEC,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int avformat_load(void)
{
  static const ffchar_t AVFORMAT[]=FF_AVFORMAT;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVFORMAT=(sizeof AVFORMAT)/(sizeof AVFORMAT[0]),
  };

  if (avutil_load()<0)
    return -1;
  else if (avcodec_load()<0)
    return -1;
  else if (!avformat.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVFORMAT)) {
      FFVMESSAGE("loading %s",AVFORMAT);
      return -1;
    }

    FFSTRCPY(pp,AVFORMAT);
    avformat.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!avformat.hLib&&!(avformat.hLib=FFDLOPEN(AVFORMAT,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int swresample_load(void)
{
  static const ffchar_t SWRESAMPLE[]=FF_SWRESAMPLE;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_SWRESAMPLE=(sizeof SWRESAMPLE)/(sizeof SWRESAMPLE[0]),
  };

  if (avutil_load()<0)
    return -1;
  else if (!swresample.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_SWRESAMPLE)) {
      FFVMESSAGE("loading %s",SWRESAMPLE);
      return -1;
   }

    FFSTRCPY(pp,SWRESAMPLE);
    swresample.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!swresample.hLib
        &&!(swresample.hLib=FFDLOPEN(SWRESAMPLE,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int swscale_load(void)
{
  static const ffchar_t SWSCALE[]=FF_SWSCALE;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_SWSCALE=(sizeof SWSCALE)/(sizeof SWSCALE[0]),
  };

  if (avutil_load()<0)
    return -1;
  else if (!swscale.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_SWSCALE)) {
      FFVMESSAGE("loading %s",SWSCALE);
      return -1;
   }

    FFSTRCPY(pp,SWSCALE);
    swscale.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!swscale.hLib&&!(swscale.hLib=FFDLOPEN(SWSCALE,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int postproc_load(void)
{
  static const ffchar_t POSTPROC[]=FF_POSTPROC;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_POSTPROC=(sizeof POSTPROC)/(sizeof POSTPROC[0]),
  };

  if (avutil_load()<0)
    return -1;
  else if (!postproc.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_POSTPROC)) {
      FFVMESSAGE("loading %s",POSTPROC);
      return -1;
    }

    FFSTRCPY(pp,POSTPROC);
    postproc.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!postproc.hLib&&!(postproc.hLib=FFDLOPEN(POSTPROC,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

static int avfilter_load(void)
{
  static const ffchar_t AVFILTER[]=FF_AVFILTER;

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVFILTER=(sizeof AVFILTER)/(sizeof AVFILTER[0]),
  };

  if (avcodec_load()<0)
    return -1;
  else if (avformat_load()<0)
    return -1;
  else if (avutil_load()<0)
    return -1;
  else if (postproc_load()<0)
    return -1;
  else if (swresample_load()<0)
    return -1;
  else if (swscale_load()<0)
    return -1;
  else if (!avfilter.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVFILTER)) {
      FFVMESSAGE("loading %s",AVFILTER);
      return -1;
    }

    FFSTRCPY(pp,AVFILTER);
    avfilter.hLib=FFDLOPEN(path,DLOPEN_FLAG);

    if (!avfilter.hLib&&!(avfilter.hLib=FFDLOPEN(AVFILTER,DLOPEN_FLAG))) {
      FFVMESSAGE("loading %s",path);
      return -1;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
static int avutil_load_sym(void *p, const char *sym)
{
  if (*(char **)p)
    return 0;
  else if (avutil_load()<0)
    return -1;
  else
    return load(avutil.hLib,sym,p);
}

static int avcodec_load_sym(void *p, const char *sym)
{
  if (*(char **)p)
    return 0;
  else if (avcodec_load()<0)
    return -1;
  else
    return load(avcodec.hLib,sym,p);
}

static int avformat_load_sym(void *p, const char *sym)
{
  if (*(char **)p)
    return 0;
  else if (avformat_load()<0)
    return -1;
  else
    return load(avformat.hLib,sym,p);
}

static int swresample_load_sym(void *p, const char *sym)
{
  if (*(char **)p)
    return 0;
  else if (swresample_load()<0)
    return -1;
  else
    return load(swresample.hLib,sym,p);
}

static int swscale_load_sym(void *p, const char *sym)
{
  if (*(char **)p)
    return 0;
  else if (swscale_load()<0)
    return -1;
  else
    return load(swscale.hLib,sym,p);
}

static int postproc_load_sym(void *p FFUNUSED, const char *sym FFUNUSED)
{
  if (*(char **)p)
    return 0;
  else if (postproc_load()<0)
    return -1;
  else
    return load(postproc.hLib,sym,p);
}

static int avfilter_load_sym(void *p FFUNUSED, const char *sym FFUNUSED)
{
  if (*(char **)p)
    return 0;
  else if (avfilter_load()<0)
    return -1;
  else
    return load(avfilter.hLib,sym,p);
}

///////////////////////////////////////////////////////////////////////////////
static int ff_dynload_absolute(const ffchar_t *dirname)
{
  enum { SIZE=(sizeof path)/(sizeof path[0]) };
  int code=-1;
  ffchar_t *mp=path+SIZE;
	int size=FFSTRLEN(dirname)+2;

  if (mp<=pp+size+1)
    goto exit;

	FFSTRCPY(path,dirname);
  pp+=size-1;
  *pp++=FFL('\\');
  *pp=FFL('\0');

  /////////////////////////////////////////////////////////////////////////////
  code=0;
exit:
  return code;
}

static int ff_dynload_relative(const ffchar_t *dirname)
{
  enum { SIZE=(sizeof path)/(sizeof path[0]) };
  int code=-1;
  ffchar_t *mp=path+SIZE;
#if ! defined (_WIN32) // [
  char process_path[64];
#endif // ]
  int len,size;

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  len=GetModuleFileNameW(NULL,pp,mp-pp);

	if (ERROR_INSUFFICIENT_BUFFER==GetLastError())
    goto exit;
#else // ] [
  sprintf(process_path,"/proc/%d/exe",getpid());

  if ((len=readlink(process_path,path,SIZE-1))<0)
    goto exit;

	len=strlen(pp);
  pp=path+len;
  *pp='\0';
#endif // ]

  if (mp<=pp+len+1)
    goto exit;

  pp+=len;

  /////////////////////////////////////////////////////////////////////////////
  while (path<pp&&FFL('/')!=pp[-1]&&FFL('\\')!=pp[-1])
    --pp;

	size=FFSTRLEN(dirname)+1;

  if (mp<=pp+size+1)
    goto exit;

	FFSTRCPY(pp,dirname);

  pp+=size-1;
#if defined (_WIN32) // [
  *pp++=FFL('\\');
#else // ] [
  *pp++=FFL('/');
#endif // ]
  *pp=FFL('\0');

  /////////////////////////////////////////////////////////////////////////////
  code=0;
exit:
  return code;
}

void ff_unload(void);

const char *ff_dynload_path(void)
{
#if defined (_WIN32) // [
  static char patha[PATH_MAX];
  const char *lang=getenv("LANG");
  UINT uCodePage=!lang||!strstr(lang,"UTF-8")?CP_OEMCP:CP_UTF8;
#endif // ]

  *pp=FFL('\0');

#if defined (_WIN32) // [
  WideCharToMultiByte(
    uCodePage,        // UINT                               CodePage,
    0ul,              // DWORD                              dwFlags,
    path,             // _In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr,
    -1,               // int                                cchWideChar,
    patha,            // LPSTR                              lpMultiByteStr,
    sizeof patha,     // int                                cbMultiByte,
    NULL,             // LPCCH                              lpDefaultChar,
    NULL              // LPBOOL                             lpUsedDefaultChar
  );

  return patha;
#else // ] [
  return path;
#endif // ]
}

int ff_dynload(const ffchar_t *dirname)
{
  int code=-1;

  if (NULL==dirname||'/'==dirname[0])
    code=ff_dynload_absolute(dirname);
  else if ('\\'==dirname[0]||(dirname[0]!=0&&dirname[1]==':'))
    code=ff_dynload_absolute(dirname);
  else
    code=ff_dynload_relative(dirname);

  if (code<0) {
#if defined (_WIN32) // [
    DVMESSAGEW(L"setting dirname \"%s\"",dirname);
#else // ] [
    DVMESSAGE("setting dirname \"%s\"",dirname);
#endif // ]
    goto exit;
  }

  return 0;
//cleanup:
  ff_unload();
exit:
  return -1;
}

void ff_unload(void)
{
  if (avfilter.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avfilter.hLib);
#else // ] [
    dlclose(avfilter.hLib);
#endif // ]
	}

  if (postproc.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(postproc.hLib);
#else // ] [
    dlclose(postproc.hLib);
#endif // ]
	}

  if (swscale.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(swscale.hLib);
#else // ] [
    dlclose(swscale.hLib);
#endif // ]
	}

  if (swresample.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(swresample.hLib);
#else // ] [
    dlclose(swresample.hLib);
#endif // ]
	}

  if (avformat.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avformat.hLib);
#else // ] [
    dlclose(avformat.hLib);
#endif // ]
	}

  if (avcodec.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avcodec.hLib);
#else // ] [
    dlclose(avcodec.hLib);
#endif // ]
	}

  if (avutil.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avutil.hLib);
#else // ] [
    dlclose(avutil.hLib);
#endif // ]
	}
}
//#else // ] [
//#error 55
#endif // ]
