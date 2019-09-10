/*
 * ff.h
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
#if ! defined (__FF_H__) // [
#define __FF_H__
#if defined (HAVE_CONFIG_H) // [
#include <config.h>
#endif // ]
#if defined (_WIN32) // [
#include <windows.h>
#endif // ]
#include <stdio.h>
#include <pbutil_priv.h>
#include <libavutil/audio_fifo.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
#include <libavfilter/avfilter.h>

#if defined (__cplusplus) // [
extern "C" {
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
#define FF_OPTION_UTF16
#define FF_GETOPT_LONG(argc,argv,optstring,longopts,longindex) \
    getoptW_long(argc,argv,optstring,longopts,longindex)
#define FFL(x) L ## x
#define _FFSTR(x) L ## x
#define FFSTR(x) _FFSTR(x)
#define FFSEPCHAR FFL('\\')
#define FFBASENAME(s) pbu_wbasename(s)
#define FFISDIGIT(ch) iswdigit(ch)
#define FFATOI(x) _wtoi(x)
#define FFATOF(x) _wtof(x)
#define FFSTRCASECMP(s1,s2) wcsicmp(s1,s2)
#define FFSTRDUP(s) _wcsdup(s)
#define FFSTRLEN(s) wcslen(s)
#define FFSTRCMP(s1,s2) wcscmp(s1,s2)
#define FFSTRCPY(s1,s2) wcscpy(s1,s2)
#define FFSTRSTR(s1,s2) wcsstr(s1,s2)
#define FFFOPEN(path,mode) _wfopen(path,mode)
#define _FFPUTC(c,f) fputwc(c,f)
#define FFPUTC(c,f) _FFPUTC(L##c,f)
#define _FFPUTS(s,f) fputws(s,f)
#define FFPUTS(s,f) _FFPUTS(L##s,f)
#define _FFPRINTF(f,format,...) fwprintf(f,format,__VA_ARGS__)
#define FFPRINTF(f,format,...) _FFPRINTF(f,L##format,__VA_ARGS__)
#define FFSPRINTF(s,size,format,...) swprintf(s,size,format,__VA_ARGS__)
#define FFSNPRINTF(s,size,format,...) snwprintf(s,size,format,__VA_ARGS__)
#define FFMESSAGE(m) PBU_DMESSAGEW(L##m)
#define FFVMESSAGE(m,...) PBU_DVMESSAGEW(L##m,__VA_ARGS__)
#define FFWARNING(m) PBU_DWARNINGW(L##m)
#define FFVWARNING(m,...) PBU_DVWARNINGW(L##m,__VA_ARGS__)
#define FFVWRITELN(m,...) PBU_DVWRITELNW(L##m,__VA_ARGS__)
#define FFDLOPEN(path,flag) LoadLibraryW(path)
#define FFDLSYM(handle,symbol) GetProcAddress(handle,symbol)
#define FFSTRTOK_R(str,delim,saveptr) wcstok(str,delim)
#if ! defined (PATH_MAX) // [
#define PATH_MAX MAX_PATH
#endif // ]

typedef wchar_t ffchar_t;
#else // ] [
#define FF_GETOPT_LONG(argc,argv,optstring,longopts,longindex) \
    getopt_long(argc,argv,optstring,longopts,longindex)
#define FFL(x) x
#define FFSTR(x) x
#define FFBASENAME(s) pbu_basename(s)
#define FFSEPCHAR FFL('/')
#define FFISDIGIT(ch) isdigit(ch)
#define FFATOI(x) atoi(x)
#define FFATOF(x) atof(x)
#if defined (_MSC_VER) // [
#define FFSTRCASECMP(s1,s2) stricmp(s1,s2)
#else // ] [
#define FFSTRCASECMP(s1,s2) strcasecmp(s1,s2)
#endif // ]
#define FFSTRDUP(s) strdup(s)
#define FFSTRLEN(s) strlen(s)
#define FFSTRCMP(s1,s2) strcmp(s1,s2)
#define FFSTRCPY(s1,s2) strcpy(s1,s2)
#define FFSTRSTR(s1,s2) strstr(s1,s2)
#define FFFOPEN(path,mode) fopen(path,mode)
#define _FFPUTC(s,f) fputc(s,f)
#define FFPUTC(s,f) _FFPUTC(s,f)
#define _FFPUTS(s,f) fputs(s,f)
#define FFPUTS(s,f) _FFPUTS(s,f)
#define _FFPRINTF(f,format,...) fprintf(f,format,__VA_ARGS__)
#define FFPRINTF(f,format,...) _FFPRINTF(f,format,__VA_ARGS__)
#define FFSPRINTF(s,size,format,...) sprintf(s,format,__VA_ARGS__)
#define FFSNPRINTF(s,size,format,...) snprintf(s,size,format,__VA_ARGS__)
#define FFMESSAGE(m) PBU_DMESSAGE(m)
#define FFVMESSAGE(m,...) PBU_DVMESSAGE(m,__VA_ARGS__)
#define FFWARNING(m) PBU_DWARNING(m)
#define FFVWARNING(m,...) PBU_DVWARNING(m,__VA_ARGS__)
#define FFVWRITELN(m,...) PBU_DVWRITELN(m,__VA_ARGS__)
#define FFDLOPEN(path,flag) dlopen(path,flag)
#define FFDLSYM(handle,symbol) dlsym(handle,symbol)
#define FFSTRTOK_R(str,delim,saveptr) strtok_r(str,delim,saveptr)

typedef char ffchar_t;
#endif // ]

#define FFPATHSEP FFSEPCHAR

///////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__ // [
#define FFUNUSED __attribute__((__unused__))
#else // ] [
#define FFUNUSED
#endif // ]

#if defined (HAVE_FF_DYNLOAD) // [
///////////////////////////////////////////////////////////////////////////////
#if ! defined (FF_AVUTIL_V) // [
  #define FF_AVUTIL_V PBU_STR(LIBAVUTIL_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_AVCODEC_V) // [
  #define FF_AVCODEC_V PBU_STR(LIBAVCODEC_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_AVFORMAT_V) // [
  #define FF_AVFORMAT_V PBU_STR(LIBAVFORMAT_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_SWRESAMPLE_V) // [
  #define FF_SWRESAMPLE_V PBU_STR(LIBSWRESAMPLE_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_SWSCALE_V) // [
  #define FF_SWSCALE_V PBU_STR(LIBSWSCALE_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_POSTPROC_V) // [
  #define FF_POSTPROC_V PBU_STR(LIBPOSTPROC_VERSION_MAJOR)
#endif // ]
#if ! defined (FF_AVFILTER_V) // [
  #define FF_AVFILTER_V PBU_STR(LIBAVFILTER_VERSION_MAJOR)
#endif // ]

#if defined (_WIN32) // [
#if defined (FF_AVUTIL_V) // [
#define FF_AVUTIL L"avutil-" PBU_WIDEN(FF_AVUTIL_V) L".dll"
#else // ] [
#define FF_AVUTIL L"avutil.dll"
#endif // ]
#if defined (FF_AVCODEC_V) // [
#define FF_AVCODEC L"avcodec-" PBU_WIDEN(FF_AVCODEC_V)  L".dll"
#else // ] [
#define FF_AVCODEC L"avcodec.dll"
#endif // ]
#if defined (FF_AVFORMAT_V) // [
#define FF_AVFORMAT L"avformat-" PBU_WIDEN(FF_AVFORMAT_V)  L".dll"
#else // ] [
#define FF_AVFORMAT L"avformat.dll"
#endif // ]
#if defined (FF_SWRESAMPLE_V) // [
#define FF_SWRESAMPLE L"swresample-" PBU_WIDEN(FF_SWRESAMPLE_V)  L".dll"
#else // ] [
#define FF_SWRESAMPLE L"swresample.dll"
#endif // ]
#if defined (FF_SWSCALE_V) // [
#define FF_SWSCALE L"swscale-" PBU_WIDEN(FF_SWSCALE_V)  L".dll"
#else // ] [
#define FF_SWSCALE L"swscale.dll"
#endif // ]
#if defined (FF_POSTPROC_V) // [
#define FF_POSTPROC L"postproc-" PBU_WIDEN(FF_POSTPROC_V)  L".dll"
#else // ] [
#define FF_POSTPROC L"postproc.dll"
#endif // ]
#if defined (FF_AVFILTER_V) // [
#define FF_AVFILTER L"avfilter-" PBU_WIDEN(FF_AVFILTER_V)  L".dll"
#else // ] [
#define FF_AVFILTER L"avfilter.dll"
#endif // ]
#else // ] [
#if defined (FF_AVUTIL_V) // [
#define FF_AVUTIL "libavutil.so." FF_AVUTIL_V
#else // ] [
#define FF_AVUTIL "libavutil.so"
#endif // ]
#if defined (FF_AVCODEC_V) // [
#define FF_AVCODEC "libavcodec.so." FF_AVCODEC_V
#else // ] [
#define FF_AVCODEC "libavcodec.so"
#endif // ]
#if defined (FF_AVFORMAT_V) // [
#define FF_AVFORMAT "libavformat.so." FF_AVFORMAT_V
#else // ] [
#define FF_AVFORMAT "libavformat.so"
#endif // ]
#if defined (FF_SWRESAMPLE_V) // [
#define FF_SWRESAMPLE "libswresample.so." FF_SWRESAMPLE_V
#else // ] [
#define FF_SWRESAMPLE "libswresample.so"
#endif // ]
#if defined (FF_SWSCALE_V) // [
#define FF_SWSCALE "libswscale.so." FF_SWSCALE_V
#else // ] [
#define FF_SWSCALE "libswscale.so"
#endif // ]
#if defined (FF_POSTPROC_V) // [
#define FF_POSTPROC "libpostproc.so." FF_POSTPROC_V
#else // ] [
#define FF_POSTPROC "libpostproc.so"
#endif // ]
#if defined (FF_AVFILTER_V) // [
#define FF_AVFILTER "libavfilter.so." FF_AVFILTER_V
#else // ] [
#define FF_AVFILTER "libavfilter.so"
#endif // ]
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int ff_dynload(const ffchar_t *dirname);
void ff_unload(void);

const char *ff_dynload_path(void);
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#define FF_PACKET_UNREF
#define FF_FRAME_UNREF
// as it seems it is impossible to pass through a partial flac stream
// (cf. "https://trac.ffmpeg.org/ticket/7864".) our hack consists in
// reading/decoding and re-encoding/writing.
#define FF_FLAC_HACK
#define FF_STREAM_METADATA

#if defined (HAVE_FF_DYNLOAD) // [
///////////////////////////////////////////////////////////////////////////////
// avfilter
//   avcodec
//     avutil
//     swresample
//   avformat
//     avutil
//     avcodec
//   avutil
//   postproc
//     avutil
//   swresample
//     avutil
//   swscale
//     avutil
//
// avutil
// swresample
// avcodec
// avformat
// swscale
// postproc
// avfilter
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
int ff_csv2avdict(const char *file, const wchar_t *filew, char sep,
    AVDictionary **metadata, int erropen);
#else // ] [
int ff_csv2avdict(const char *file, char sep,
    AVDictionary **metadata, int erropen);
#endif // ]
#if defined (_WIN32) // [
char *ff_wcs2str(const wchar_t *wcs, char *buf, int codepage, size_t size);
#endif // ]
int ff_fexists(const ffchar_t *path);
int ff_fcmp(const ffchar_t *lpath, const ffchar_t *rpath);
int ff_rm(const ffchar_t *path);
int ff_mv(const ffchar_t *source, const ffchar_t *target);
int ff_mkdir(ffchar_t *path);

///////////////////////////////////////////////////////////////////////////////
typedef const struct ff_iter_vmt ff_iter_vmt_t;
typedef struct ff_iter ff_iter_t;
typedef struct ff_param_decode ff_param_decode_t;
typedef struct ff_audio ff_audio_t;
typedef const struct ff_input_callback ff_input_callback_t;
#if defined (FF_STREAM_METADATA) // [
typedef enum ff_metadata_type ff_metadata_type_t;
#endif // ]
typedef const struct ff_output_callback ff_output_callback_t;
typedef struct ff_inout ff_inout_t;
typedef struct ff_resampler ff_resampler_t;
typedef enum ff_analyzer_state ff_analyzer_state_t;
typedef struct ff_analyzer ff_analyzer_t;
typedef struct ff_filter ff_filter_t;
typedef struct ff_fifo ff_fifo_t;
typedef enum ff_muxer_state ff_muxer_state_t;
typedef struct ff_muxer ff_muxer_t;
typedef struct ff_printer ff_printer_t;

///////////////////////////////////////////////////////////////////////////////
struct ff_iter_vmt {
  const char *id;
  void (*first)(ff_iter_t *i);
  int (*valid)(ff_iter_t *i);
  void (*next)(ff_iter_t *i);
  void (*norm)(ff_iter_t *i, double *x, double *max);
};

struct ff_iter {
  ff_iter_vmt_t *vmt;
  const AVFrame *frame;

	union {
    // interleaved ////////////////////////////////////////////////////////////
    union {
      union {
        struct { int16_t *rp,*mp; } s16;
        struct { int32_t *rp,*mp; } s32;
        struct { float   *rp,*mp; } flt;
        struct { double  *rp,*mp; } dbl;
      };
    } i;

    // planar /////////////////////////////////////////////////////////////////
    union {
      union {
        struct { int16_t *rp[AV_NUM_DATA_POINTERS],*mp; } s16;
        struct { int32_t *rp[AV_NUM_DATA_POINTERS],*mp; } s32;
        struct { float   *rp[AV_NUM_DATA_POINTERS],*mp; } flt;
        struct { double  *rp[AV_NUM_DATA_POINTERS],*mp; } dbl;
      };
    } p;
  };
};

int ff_iter_first(ff_iter_t *i, const AVFrame *frame);

///////////////////////////////////////////////////////////////////////////////
struct ff_param_decode {
  struct {
	  enum AVSampleFormat sample_fmt;
    int64_t channel_layout;
  } request;

  struct {
    int enabled;
    double scale;
  } drc;
};

///////////////////////////////////////////////////////////////////////////////
struct ff_audio {
  AVCodecContext *ctx;
};

int ff_audio_create(ff_audio_t *audio, ff_inout_t *inout,
    const ff_param_decode_t *iparam,
    const AVCodecParameters *ocodecpar);
void ff_audio_destroy(ff_audio_t *audio);

///////////////////////////////////////////////////////////////////////////////
// interface implemented in e.g. "bg_track.c".
struct ff_input_callback {
  const char *(*path)(const void *data);
#if defined (_WIN32) // [
  const wchar_t *(*pathw)(const void *data);
#endif // ]
  const ff_param_decode_t *(*decode)(const void *data);
  int (*upsample)(const void *data);
  int (*transcode)(const void *data);
  int (*csv)(const void *data);
  int (*suppress_progress)(const void *data);

  struct {
    int64_t (*begin)(const void *data);
    int64_t (*duration)(const void *data);
  } interval;

  struct {
    int (*create)(void *data, const AVCodecParameters *codecpar);
    void (*destroy)(void *data);
    int (*add)(void *data, int upsampled, AVFrame *frame);
  } stats;
};

// interface implemented in e.g. "bg_muxer.c".
#if defined (FF_STREAM_METADATA) // [
enum ff_metadata_type {
	FF_METADATA_TYPE_FORMAT,
	FF_METADATA_TYPE_AUDIO,
	FF_METADATA_TYPE_VIDEO,
};
#endif // ]

struct ff_output_callback {
  const char *(*path)(const void *data);
  const ff_analyzer_t *(*analyzer)(const void *data);
  enum AVSampleFormat (*sample_fmt)(const void *data);
#if defined (FF_STREAM_METADATA) // [
  void (*metadata)(void *data, AVDictionary **om, const AVDictionary *im,
			ff_metadata_type_t type);
#else // ] [
  void (*metadata)(void *data, AVDictionary **om, const AVDictionary *im);
#endif // ]
  // just called from the muxer.
  enum AVCodecID (*codec_id)(const void *data, const AVOutputFormat *oformat);
};

struct ff_printer {
  FILE *f;
  size_t len;
};

int ff_printer_create(ff_printer_t *p, FILE *f);
void ff_printer_destroy(ff_printer_t *p);

void ff_printer_clear(ff_printer_t *p);
void ff_printer_reset(ff_printer_t *p);
void ff_printer_flush(ff_printer_t *p);

int ff_printer_printf(ff_printer_t *p, const char *format, ...);
#if defined (_WIN32) // [
int ff_printer_wprintf(ff_printer_t *p, const wchar_t *format, ...);
#endif // ]

struct ff_inout {
  struct {
    void *data;

    union {
      ff_input_callback_t *in;
      ff_output_callback_t *out;
    };
  } cb;

  ff_printer_t *printer;

  struct {
    AVFormatContext *ctx;
  } fmt;

  int ai,vi;
  ff_audio_t audio;
};

////////
int ff_input_create(ff_inout_t *in, ff_input_callback_t *cb, void *data,
    int warn, ff_printer_t *p);
void ff_input_destroy(ff_inout_t *in);

// for re-opening for a second time, e.g. for the purpose of re-muxing:
int ff_input_open_analyzer(ff_inout_t *in);
#if defined (FF_FLAC_HACK) // [
int ff_input_open_muxer(ff_inout_t *in, int *hack);
#else // ] [
int ff_input_open_muxer(ff_inout_t *in);
#endif // ]
void ff_input_close(ff_inout_t *in);

void ff_inout_interval(ff_inout_t *inout, int64_t *begin, int64_t *duration,
    AVStream *stream);
int ff_input_progress(ff_inout_t *in, AVPacket *pkt);

////////
#if defined (FF_FLAC_HACK) // [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *ocb, void *odata,
    enum AVSampleFormat sample_fmt);
#else // ] [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *cb, void *data);
#endif // ]
void ff_output_destroy(ff_inout_t *out);

///////////////////////////////////////////////////////////////////////////////
struct ff_resampler {
  SwrContext *ctx;
  AVFrame *frame;
};

int ff_resampler_create(ff_resampler_t *res,
    const AVCodecParameters *ocodecpar,
    const AVCodecParameters *icodecpar);
void resampler_destroy(ff_resampler_t *res);
int resampler_apply(ff_resampler_t *res, AVFrame *frame);

///////////////////////////////////////////////////////////////////////////////
enum ff_analyzer_state {
  FF_ANALYZER_DECODER_SEND_PACKET,
  FF_ANALYZER_DECODER_RECEIVE_FRAME,
};

struct ff_analyzer {
  ff_analyzer_state_t state;
  ff_inout_t *in;
  ff_resampler_t normalizer;
  ff_resampler_t upsampler;
  AVPacket *pkt;
  AVFrame *frame;
};

int ff_analyzer_create(ff_analyzer_t *a, ff_inout_t *in);
void ff_analyzer_destroy(ff_analyzer_t *a, int destroy_stats);
int ff_analyzer_loop(ff_analyzer_t *analyzer);

///////////////////////////////////////////////////////////////////////////////
struct ff_filter {
  struct {
    AVFilterContext *sink,*src;
  } ctx;

  AVFilterGraph *graph;
};

int ff_filter_create(ff_filter_t *filter,
    const AVCodecParameters *ocodecpar,
    const AVCodecParameters *icodecpar,
    AVRational time_base, const char *descr);
void ff_filter_destroy(ff_filter_t *filter);

int ff_filter_send_frame(ff_filter_t *filter, AVFrame *frame);
int ff_filter_receive_frame(ff_filter_t *filter, AVFrame *frame);

///////////////////////////////////////////////////////////////////////////////
// fifo not used. just for reference.
struct ff_fifo {
  AVCodecParameters ocodecpar;
  int flush;
  AVAudioFifo *fifo;
};

int ff_fifo_create(ff_fifo_t *fifo, const AVCodecParameters *ocodecpar);
void ff_fifo_destroy(ff_fifo_t *fifo);

int ff_fifo_size(ff_fifo_t *fifo);
int ff_fifo_send_frame(ff_fifo_t *fifo, AVFrame *frame);
int ff_fifo_receive_frame(ff_fifo_t *fifo, AVFrame *frame);

///////////////////////////////////////////////////////////////////////////////
enum ff_muxer_state {
  FF_MUXER_DECODER_SEND_PACKET,
  FF_MUXER_DECODER_RECEIVE_FRAME,
  FF_MUXER_FILTER_SEND_FRAME,
  FF_MUXER_FILTER_RECEIVE_FRAME,
  FF_MUXER_ENCODER_SEND_FRAME,
  FF_MUXER_ENCODER_RECEIVE_PACKET,
};

struct ff_muxer {
  ff_muxer_state_t state;
  ff_inout_t *in,*out;
  AVPacket *pkt;
  ff_filter_t filter;
  AVFrame *frame;

  struct {
    int64_t ts;
  } audio;
};

int ff_muxer_create(ff_muxer_t *muxer, ff_inout_t *in, ff_inout_t *out,
    const char *filter);
void ff_muxer_destroy(ff_muxer_t *muxer);

int ff_muxer_loop(ff_muxer_t *muxer);

#if defined (__cplusplus) // [
}
#endif // ]
#endif // __FF_H__ ]
