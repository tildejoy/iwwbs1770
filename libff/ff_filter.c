/*
 * ff_filter.c
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
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <ff.h>

///////////////////////////////////////////////////////////////////////////////
int ff_filter_create(ff_filter_t *f,
    const AVCodecParameters *ocodecpar,
    const AVCodecParameters *icodecpar,
    AVRational time_base, const char *descr)
{
  // cf. "<ffmpeg>/doc/examples/filtering_audio.c".
  int err=0;
  const enum AVSampleFormat sink_sample_fmts[]
      ={ ocodecpar->format,-1 };
  const int64_t sink_channel_layouts[]
      ={ ocodecpar->channel_layout,-1 };
  const int sink_sample_rates[]
      ={ ocodecpar->sample_rate,-1 };
  char args[512];
  AVCodec *ocodec;

  struct {
    const AVFilter *f;
    AVFilterInOut *in;
  } src;

  struct {
    const AVFilter *f;
    AVFilterInOut *out;
  } sink;

  /////////////////////////////////////////////////////////////////////////////
  f->graph=avfilter_graph_alloc();

  if (!f->graph) {
    DMESSAGE("allocating filter graph");
    err=AVERROR(ENOMEM);
    goto e_graph;
  }

  /////////////////////////////////////////////////////////////////////////////
  // buffer audio source: the decoded frames from the decoder will
  // be inserted here.
  src.f=avfilter_get_by_name("abuffer");

  if (!src.f) {
    DMESSAGE("audio filter \"abuffer\" not available");
    err=-1;
    goto e_src;
  }

  /////////////////////////////////////////////////////////////////////////////
  snprintf(args,sizeof args,
      "time_base=%d/%d"
      ":sample_rate=%d"
      ":sample_fmt=%s"
      ":channel_layout=0x%"PRIx64,
      time_base.num,
      time_base.den,
      icodecpar->sample_rate,
      av_get_sample_fmt_name(icodecpar->format),
      icodecpar->channel_layout);
  err=avfilter_graph_create_filter(&f->ctx.src,src.f,"in",args,NULL,f->graph);

  if (err<0) {
    DVMESSAGE("creating filter source: %s (%d)",av_err2str(err),err);
    goto e_srcctx;
  }

  /////////////////////////////////////////////////////////////////////////////
  // buffer audio sink: to terminate the filter chain.
  sink.f=avfilter_get_by_name("abuffersink");

  if (!sink.f) {
    DMESSAGE("audio filter \"abuffersink\" not available");
    err=-1;
    goto e_sink;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=avfilter_graph_create_filter(&f->ctx.sink,sink.f,"out",NULL,NULL,
      f->graph);

  if (err<0) {
    DVMESSAGE("creating filter sink: %s (%d)",av_err2str(err),err);
    goto e_sinkctx;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=av_opt_set_int_list(f->ctx.sink,"sample_fmts",
      sink_sample_fmts,-1,AV_OPT_SEARCH_CHILDREN);

  if (err<0) {
    DVMESSAGE("setting output sample format: %s (%d)",av_err2str(err),err);
    goto e_sinkargs;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=av_opt_set_int_list(f->ctx.sink,"channel_layouts",
      sink_channel_layouts,-1,AV_OPT_SEARCH_CHILDREN);

  if (err<0) {
    DVMESSAGE("setting output channel layout: %s (%d)",av_err2str(err),err);
    goto e_sinkargs;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=av_opt_set_int_list(f->ctx.sink,"sample_rates",
      sink_sample_rates,-1,AV_OPT_SEARCH_CHILDREN);

  if (err<0) {
    DVMESSAGE("setting output sample rate: %s (%d)",av_err2str(err),err);
    goto e_sinkargs;
  }

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Set the endpoints for the filter graph. The filter_graph will
   * be linked to the graph described by filters_descr.
   */

  /*
   * The buffer source output must be connected to the input pad of
   * the first filter described by filters_descr; since the first
   * filter input label is not specified, it is set to "in" by
   * default.
   */
  sink.out=avfilter_inout_alloc();

  if (!sink.out) {
    DMESSAGE("allocating outputs");
    goto e_outputs;
  }

  sink.out->name=av_strdup("in");

  if (!sink.out->name) {
    DMESSAGE("duplicating sink name");
    goto e_sinkname;
  }

  sink.out->filter_ctx=f->ctx.src;
  sink.out->pad_idx=0;
  sink.out->next= NULL;

  /*
   * The buffer sink input must be connected to the output pad of
   * the last filter described by filters_descr; since the last
   * filter output label is not specified, it is set to "out" by
   * default.
   */
  src.in=avfilter_inout_alloc();

  if (!src.in) {
    DMESSAGE("allocating inputs");
    goto e_inputs;
  }

  src.in->name=av_strdup("out");

  if (!src.in->name) {
    DMESSAGE("duplicating source name");
    goto e_srcname;
  }

  src.in->filter_ctx=f->ctx.sink;
  src.in->pad_idx=0;
  src.in->next=NULL;

  /////////////////////////////////////////////////////////////////////////////
  err=avfilter_graph_parse_ptr(f->graph,descr,&src.in,
      &sink.out,NULL);

  if (err<0) {
    DVMESSAGE("parsing: %s (%d)",av_err2str(err),err);
    goto e_parse;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=avfilter_graph_config(f->graph,NULL);

  if (err<0) {
    DVMESSAGE("onfiguring: %s (%d)",av_err2str(err),err);
    goto e_config;
  }

  /////////////////////////////////////////////////////////////////////////////
  // needs to be called when the graph already has been linked.
  ocodec=avcodec_find_encoder(ocodecpar->codec_id);

  if (!ocodec) {
    DMESSAGE("target codec doesn't exist");
    goto e_ocodec;
  }

  if (!(ocodec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE))
    av_buffersink_set_frame_size(f->ctx.sink,ocodecpar->frame_size);

  /////////////////////////////////////////////////////////////////////////////
  avfilter_inout_free(&src.in);
  avfilter_inout_free(&sink.out);

  ///////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_ocodec:
e_config:
e_parse:
e_srcname:
  avfilter_inout_free(&src.in);
e_inputs:
e_sinkname:
  avfilter_inout_free(&sink.out);
e_outputs:
e_sinkargs:
e_sinkctx:
e_sink:
e_srcctx:
e_src:
  avfilter_graph_free(&f->graph);
e_graph:
  return err;
}

void ff_filter_destroy(ff_filter_t *f)
{
  avfilter_graph_free(&f->graph);
}

///////////////////////////////////////////////////////////////////////////////
int ff_filter_send_frame(ff_filter_t *f, AVFrame *frame)
{
  return av_buffersrc_add_frame(f->ctx.src,frame);
}

int ff_filter_receive_frame(ff_filter_t *f, AVFrame *frame)
{
  return av_buffersink_get_frame(f->ctx.sink,frame);
}
