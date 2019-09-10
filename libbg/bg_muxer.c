/*
 * bg_muxer.c
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
#if defined (_WIN32) // [
#include <locale.h>
#else // ] [
#include <wctype.h>
#include <wchar.h>
#include <fcntl.h>
#include <unistd.h>
#endif // ]
#include <bg.h>

///////////////////////////////////////////////////////////////////////////////
static bg_visitor_vmt_t bg_muxer_vmt;
static ff_output_callback_t bg_output_callback;

///////////////////////////////////////////////////////////////////////////////
int bg_muxer_create(bg_visitor_t *vis)
{
  /////////////////////////////////////////////////////////////////////////////
  vis->vmt=&bg_muxer_vmt;
  vis->depth=0;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
}

static void bg_muxer_destroy(bg_visitor_t *vis FFUNUSED)
{
}

///////////////////////////////////////////////////////////////////////////////
static int bg_muxer_copy_file(ffchar_t *source, ffchar_t *target)
{
  int err=-1;

#if defined (_WIN32) // [
  DeleteFileW(target);

  CopyFileW(
    source,    // LPCTSTR lpExistingFileName,
    target,    // LPCTSTR lpNewFileName,
    0          // BOOL    bFailIfExists
  );

  err=0;
#else // ] [
  // https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
  struct {
    int source,target;
  } fd;

  char buf[4096],*bp;
  ssize_t size,written;

  fd.source=open(source,O_RDONLY);

  if (fd.source<0) {
    DVMESSAGE("opening source \"%s\"",source);
    goto e_source;
  }

  remove(target);
  fd.target=open(target,O_WRONLY|O_CREAT|O_EXCL,0666);

  if (fd.target<0) {
    DVMESSAGE("opening target \"%s\"",target);
    goto e_target;
  }

  for (;;) {
    size=read(fd.source,buf,sizeof buf);

    if (size<=0)
      break;

    bp=buf;

    while (0<size) {
      written=write(fd.target,bp,size);

      if (written<=0) {
        DMESSAGE("copying");
        goto e_copy;
      }

      size-=written;
      bp+=written;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_copy:
  close(fd.target);
e_target:
  close(fd.source);
e_source:
#endif // ]
  return err;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_muxer_dispatch_file(bg_visitor_t *vis FFUNUSED, bg_tree_t *tree)
{
  int err=-1;
  bg_param_t *param=tree->param;

  if (!(BG_FLAGS_EXT_COPY&tree->param->flags.extension))
    goto success;
  else if (tree->param->overwrite)
    goto success;

  // file annotation is created in each case.
  if (param->output.dirname||param->overwrite) {
    if (tree->vmt->annotation.create(tree)<0) {
      DMESSAGE("annotating");
      goto e_annotate;
    }
  }

  if (bg_muxer_copy_file(tree->source.path,tree->temp.path)<0) {
    DMESSAGE("copying");
    goto e_copy;
  }

  ff_mv(tree->temp.path,tree->target.path);

  /////////////////////////////////////////////////////////////////////////////
success:
  err=0;
//cleanup:
e_copy:
  if (param->output.dirname||param->overwrite)
    tree->vmt->annotation.destroy(tree);
e_annotate:
  return err;
}

static int bg_muxer_dispatch_track(bg_visitor_t *vis FFUNUSED, bg_tree_t *tree)
{
  int err=-1;
  bg_track_t *track=&tree->track;
  bg_tree_t *parent=tree->parent;
  bg_param_t *param=tree->param;
  //FILE *f=param->result.f;
  int apply=BG_FLAGS_MODE_APPLY&param->flags.mode;
#if defined (FF_FLAC_HACK) // [
  int hack;
  AVCodecParameters *codecpar;
  enum AVSampleFormat sample_fmt;
#endif // ]
  ff_inout_t output;
  double gain_track,gain_album,gain,volume;
  char filter[128];
  ff_muxer_t muxer;

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_FLAC_HACK) // ] [
  if (ff_input_open_muxer(&track->input,&hack)<0) {
    DMESSAGE("re-opening input");
    goto e_input;
  }
#else // ] [
  if (ff_input_open_muxer(&track->input)<0) {
    DMESSAGE("re-opening input");
    goto e_input;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  // track annotation is created in each case.
  if (param->output.dirname||param->overwrite) {
    if (tree->vmt->annotation.create(tree)<0) {
      DMESSAGE("annotating");
      goto e_annotate;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  fprintf(stdout,"[%lu/%lu] %s\n",track->root.id,param->count.max,
      bg_tree_out_basanamen(tree));
  fflush(stdout);

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_FLAC_HACK) // [
  // as it seems it is impossible to pass through a partial flac stream
  // (cf. "https://trac.ffmpeg.org/ticket/7864".) our hack consists in
  // reading/decoding and re-encoding/writing./ hence we've need to tell
  // the output the format from decoding.
  codecpar=track->input.fmt.ctx->streams[track->input.ai]->codecpar;
  sample_fmt=hack?codecpar->format:AV_SAMPLE_FMT_NONE;

  if (ff_output_create(&output,&bg_output_callback,tree,sample_fmt)<0) {
    DMESSAGE("creating output");
    goto e_output;
  }
#else // ] [
  if (ff_output_streams_create(&output)<0) {
    DMESSAGE("creating output streams");
    goto e_output_streams;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_FLAC_HACK) // [
  apply=apply&&!hack;
#endif // ]

  if (apply) {
    if (param->weight.enabled) {
      gain_album=param->norm-lib1770_stats_get_mean(parent->stats.momentary,
          param->momentary.mean.gate);
      gain_track=param->norm-lib1770_stats_get_mean(tree->stats.momentary,
          param->momentary.mean.gate);
      gain=gain_album+param->weight.value*(gain_track-gain_album);
    }
    else {
      gain=param->norm-lib1770_stats_get_mean(parent->stats.momentary,
          param->momentary.mean.gate);
    }

    volume=LIB1770_DB2Q(gain);
    snprintf(filter,sizeof filter,"volume=%1.2f",volume);
  }

  ///////////////////////////////////////////////////////////////////////////
  if (ff_muxer_create(&muxer,&track->input,&output,apply?filter:NULL)<0) {
    DMESSAGE("creating muxer");
    goto e_muxer;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (ff_muxer_loop(&muxer)<0) {
    DMESSAGE("re-encoder looping");
    goto e_loop;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_loop:
  ff_muxer_destroy(&muxer);
e_muxer:
  ff_output_destroy(&output);
e_output:
  if (err)
    ff_rm(tree->temp.path);
  else {
    // before moving/renaming the newley created file we first must have
    // closed the muxer/output/input!!!
    ff_input_close(&track->input);

#if defined (BG_PARAM_SLEEP) // [
    if (0<param->sleep) {
#if defined (_WIN32) // [
      Sleep(param->sleep);
#else // ] [
      sleep(param->sleep);
#endif // ]
    }
#endif // ]

    err=ff_mv(tree->temp.path,tree->target.path);

    if (err<0)
      DMESSAGE("moving");
  }

  if (param->output.dirname||param->overwrite)
    tree->vmt->annotation.destroy(tree);
e_annotate:
  ff_input_close(&track->input);
e_input:
  return err;
}

static int bg_muxer_dispatch_album(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;
  bg_tree_t *cur;

  /////////////////////////////////////////////////////////////////////////////
  for (cur=tree->album.first;cur;cur=cur->next) {
    if (cur->vmt->accept(cur,vis)<0) {
      DMESSAGE("child accepting muxer");
      goto e_child;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_child:
  return err;
}

static int bg_muxer_dispatch_root(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;
  bg_tree_t *cur;

  /////////////////////////////////////////////////////////////////////////////
  for (cur=tree->album.first;cur;cur=cur->next) {
    if (cur->vmt->accept(cur,vis)<0) {
      DMESSAGE("accepting visitor");
      goto e_child;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
// cleanup:
e_child:
  return err;
}

static bg_visitor_vmt_t bg_muxer_vmt={
#if defined (PBU_DEBUG) // [
  .id="muxer",
#endif // ]
  .destroy=bg_muxer_destroy,
  .dispatch_file=bg_muxer_dispatch_file,
  .dispatch_track=bg_muxer_dispatch_track,
  .dispatch_album=bg_muxer_dispatch_album,
  .dispatch_root=bg_muxer_dispatch_root,
};

///////////////////////////////////////////////////////////////////////////////
enum bg_muxer_tag_bit {
  BG_MUXER_TAG_BIT_ALGORITHM=1ul<<0ul,
  BG_MUXER_TAG_BIT_REFERENCE_LOUDNESS=1ul<<1ul,
  BG_MUXER_TAG_BIT_TRACK_GAIN=1ul<<2ul,
  BG_MUXER_TAG_BIT_TRACK_PEAK=1ul<<3ul,
  BG_MUXER_TAG_BIT_ALBUM_GAIN=1ul<<4ul,
  BG_MUXER_TAG_BIT_ALBUM_PEAK=1ul<<5ul,
  BG_MUXER_TAG_BIT_MAX=1ul<<6ul,
};

#if defined (FF_MUXER_CALLBACK_TAG_BIT_MAX) // [
static unsigned int bg_muxer_tag_bit_max(void *data FFUNUSED)
{
  return BG_MUXER_TAG_BIT_MAX;
}
#endif // ]

static const char *bg_muxer_path(const void *data)
{
#if defined (_WIN32) // [
  return ((const bg_tree_t *)data)->track.temp.path;
#else // ] [
  return ((const bg_tree_t *)data)->temp.path;
#endif // ]
}

static const ff_analyzer_t *bg_muxer_analyzer(const void *data)
{
  return &((const bg_tree_t *)data)->track.analyzer;
}

static enum AVSampleFormat bg_muxer_sample_fmt(const void *data FFUNUSED)
{
  return AV_SAMPLE_FMT_S32;
}

#if defined (_WIN32) // [
#define STRCASECMP(s1,s2) stricmp(s1,s2)
#define STRNCASECMP(s1,s2,n) strnicmp(s1,s2,n)
#else // ] [
#define STRCASECMP(s1,s2) strcasecmp(s1,s2)
#define STRNCASECMP(s1,s2,n) strncasecmp(s1,s2,n)
#endif // ]

static void bg_tag(void *data, bg_bits_t *bits, const char *key,
    const char *ivalue, AVDictionary **om)
{
  enum {
#if 0 // [
    BG_MUXER_FLAGS=AV_DICT_IGNORE_SUFFIX,
#else // ] [
    BG_MUXER_FLAGS=0,
#endif // ]
  };

  bg_tree_t *tree=data;
  bg_tree_t *parent=tree->parent;
  bg_param_t *param=tree->param;
  size_t len=strlen(param->tag.pfx);
  char ovalue[128];
  double gain;

  if (!key) {
    DMESSAGE("missing key");
    return;
  }

  if (STRNCASECMP(param->tag.pfx,key,len)) {
    if (!ivalue) {
      DMESSAGE("missing value");
      return;
    }

    // plain copy.
    av_dict_set(om,key,ivalue,BG_MUXER_FLAGS);
  }
  else if (!STRCASECMP(key+len,"ALGORITHM")) {
    if (!(*bits&BG_MUXER_TAG_BIT_ALGORITHM)) {
      // modified copy.
      strncpy(ovalue,"BS.1770",(sizeof ovalue)-1);
      av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
      *bits|=BG_MUXER_TAG_BIT_ALGORITHM;
    }
  }
  else if (!STRCASECMP(key+len,"REFERENCE_LOUDNESS")) {
    if (!(*bits&BG_MUXER_TAG_BIT_REFERENCE_LOUDNESS)) {
      // modified copy.
      snprintf(ovalue,sizeof ovalue,"%1.1f %sFS",param->norm,
          param->unit->n.lu);
      av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
      *bits|=BG_MUXER_TAG_BIT_REFERENCE_LOUDNESS;
    }
  }
  else if (!STRCASECMP(key+len,"TRACK_GAIN")) {
    if (BG_FLAGS_MODE_TAGS_TRACK&param->flags.mode) {
      if (BG_FLAGS_MODE_APPLY&param->flags.mode||!tree->stats.momentary) {
        // suppress.
      }
      else if (!(*bits&BG_MUXER_TAG_BIT_TRACK_GAIN)) {
        // modified copy.
        gain=param->norm-lib1770_stats_get_mean(tree->stats.momentary,
            param->momentary.mean.gate);
        snprintf(ovalue,sizeof ovalue,"%1.1f %s",gain,param->unit->n.lu);
        av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
        *bits|=BG_MUXER_TAG_BIT_TRACK_GAIN;
      }
    }
  }
  else if (!STRCASECMP(key+len,"TRACK_PEAK")) {
    if (BG_FLAGS_MODE_TAGS_TRACK&param->flags.mode) {
      if ((BG_FLAGS_MODE_APPLY&param->flags.mode)
          ||!(BG_FLAGS_AGG_PEAK&param->flags.aggregate)) {
        // suppress.
      }
      else if (!(*bits&BG_MUXER_TAG_BIT_TRACK_PEAK)) {
        // modified copy.
        if (BG_FLAGS_AGG_TRUEPEAK&param->flags.aggregate)
          snprintf(ovalue,sizeof ovalue,"%f",tree->stats.truepeak);
        else
          snprintf(ovalue,sizeof ovalue,"%f",tree->stats.samplepeak);

        av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
        *bits|=BG_MUXER_TAG_BIT_TRACK_PEAK;
      }
    }
  }
  else if (!STRCASECMP(key+len,"ALBUM_GAIN")) {
    if (BG_FLAGS_MODE_TAGS_ALBUM&param->flags.mode) {
      if (BG_FLAGS_MODE_APPLY&param->flags.mode||!parent
          ||!parent->stats.momentary) {
        // suppress.
      }
      else if (!(*bits&BG_MUXER_TAG_BIT_ALBUM_GAIN)) {
        // modified copy.
        gain=param->norm-lib1770_stats_get_mean(parent->stats.momentary,
            param->momentary.mean.gate);
        snprintf(ovalue,sizeof ovalue,"%1.1f %s",gain,param->unit->n.lu);
        av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
        *bits|=BG_MUXER_TAG_BIT_ALBUM_GAIN;
      }
    }
  }
  else if (!STRCASECMP(key+len,"ALBUM_PEAK")) {
    if (BG_FLAGS_MODE_TAGS_ALBUM&param->flags.mode) {
      if ((BG_FLAGS_MODE_APPLY&param->flags.mode)||!parent
          ||!(BG_FLAGS_AGG_PEAK&param->flags.aggregate)) {
        // suppress.
      }
      else if (!(*bits&BG_MUXER_TAG_BIT_ALBUM_PEAK)) {
        // modified copy.
        if (BG_FLAGS_AGG_TRUEPEAK&param->flags.aggregate)
          snprintf(ovalue,sizeof ovalue,"%f",parent->stats.truepeak);
        else /*if (BG_FLAGS_AGG_SAMPLEPEAK&param->flags.aggregate)*/
          snprintf(ovalue,sizeof ovalue,"%f",parent->stats.samplepeak);

        av_dict_set(om,key,ovalue,BG_MUXER_FLAGS);
        *bits|=BG_MUXER_TAG_BIT_ALBUM_PEAK;
      }
    }
  }
  else {
    // suppress.
  }
}

#if defined (FF_STREAM_METADATA) // [
static void bg_muxer_metadata(void *data, AVDictionary **om,
    const AVDictionary *im, ff_metadata_type_t type FFUNUSED)
#else // ] [
static void bg_muxer_metadata(void *data, AVDictionary **om,
    const AVDictionary *im)
#endif // ]
{
  bg_param_t *param=((bg_tree_t *)data)->param;
  size_t len=strlen(param->tag.pfx);
  const AVDictionaryEntry *opt=NULL;
  bg_bits_t bits=0ul;
  char key[128];

#if 0 // [
  // it makes no sense to first copy all tags over.
  for (;;) {
    opt=av_dict_get(im,"",opt,AV_DICT_IGNORE_SUFFIX);

    if (!opt)
      break;

    bg_tag(data,&bits,opt->key,opt->value,om);
  }
#endif // ]

#if defined (FF_STREAM_METADATA) // [
  if (FF_METADATA_TYPE_VIDEO!=type) {
#endif // ]
    // set tags with results from analysis.
    strcpy(key,param->tag.pfx);

    //if (!(BG_MUXER_TAG_BIT_ALGORITHM&bits)) {
      strcpy(key+len,"ALGORITHM");
      bg_tag(data,&bits,key,NULL,om);
    //}

    //if (!(BG_MUXER_TAG_BIT_REFERENCE_LOUDNESS&bits)) {
      strcpy(key+len,"REFERENCE_LOUDNESS");
      bg_tag(data,&bits,key,NULL,om);
    //}

    //if (!(BG_MUXER_TAG_BIT_TRACK_GAIN&bits)) {
      strcpy(key+len,"TRACK_GAIN");
      bg_tag(data,&bits,key,NULL,om);
    //}

    //if (!(BG_MUXER_TAG_BIT_TRACK_PEAK&bits)) {
      strcpy(key+len,"TRACK_PEAK");
      bg_tag(data,&bits,key,NULL,om);
    //}

    //if (!(BG_MUXER_TAG_BIT_ALBUM_GAIN&bits)) {
      strcpy(key+len,"ALBUM_GAIN");
      bg_tag(data,&bits,key,NULL,om);
    //}

    //if (!(BG_MUXER_TAG_BIT_ALBUM_PEAK&bits)) {
      strcpy(key+len,"ALBUM_PEAK");
      bg_tag(data,&bits,key,NULL,om);
    //}
#if defined (FF_STREAM_METADATA) // [
  }
#endif // ]

#if 1 // [
  // it makes make more sense to copy remaining tags over.
  for (;;) {
    opt=av_dict_get(im,"",opt,AV_DICT_IGNORE_SUFFIX);

    if (!opt)
      break;

    bg_tag(data,&bits,opt->key,opt->value,om);
  }
#endif // ]
}

static enum AVCodecID bg_muxer_codec_id(const void *data,
    const AVOutputFormat *oformat)
{
  const bg_tree_t *tree=(const bg_tree_t *)data;
  const bg_param_t *param=tree->param;
  AVCodec *codec;
  
  if (param->codec.name&&*param->codec.name) {
    codec=avcodec_find_encoder_by_name(param->codec.name);

    if (!codec) {
      DVWARNING("audio codec \"%s\" does not exist;"
          " falling back to \"FLAC\"",param->codec.name);
      return AV_CODEC_ID_FLAC;
    }
    else if (codec->id<AV_CODEC_ID_FIRST_AUDIO) {
      DVWARNING("codec \"%s\" is not an audio codec;"
          " falling back to \"FLAC\"",param->codec.name);
      return AV_CODEC_ID_FLAC;
    }
    else if (AV_CODEC_ID_FIRST_SUBTITLE<=codec->id) {
      DVWARNING("codec \"%s\" is not an audio codec;"
          " falling back to \"FLAC\"",param->codec.name);
      return AV_CODEC_ID_FLAC;
    }
    else
      return codec->id;
  }
  else {
    if (!param->out.sfx&&avformat_query_codec(oformat,AV_CODEC_ID_FLAC,
        FF_COMPLIANCE_EXPERIMENTAL)) {
      return AV_CODEC_ID_FLAC;
    }

    return oformat->audio_codec;
  }
}

static ff_output_callback_t bg_output_callback={
  .path=bg_muxer_path,
  .analyzer=bg_muxer_analyzer,
  .sample_fmt=bg_muxer_sample_fmt,
  .metadata=bg_muxer_metadata,
  .codec_id=bg_muxer_codec_id,
};
