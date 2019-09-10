/*
 * ff_audio.c
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
//#define FF_OUTPUT_BIT_RATE 96000

///////////////////////////////////////////////////////////////////////////////
int ff_audio_create(ff_audio_t *audio, ff_inout_t *inout,
    const ff_param_decode_t *iparam,
    const AVCodecParameters *ocodecpar)
{
  AVStream *stream=inout->fmt.ctx->streams[inout->ai];
  AVCodecParameters *codecpar=stream->codecpar;
  AVDictionary *opt=NULL;
  AVCodec *codec;
  int i;
  char value[64];
  int err;

  ////////////////////////////////////////////////////////////////////////////
	if ((iparam&&ocodecpar)||(!iparam&&!ocodecpar)) {
		DMESSAGE("unexpected arguments");
		goto e_args;
	}

  ////////////////////////////////////////////////////////////////////////////
  if (ocodecpar) {
    // we're going to encode and will construct the corresponding
    // stream's codec parameters below.
    codec=avcodec_find_encoder(ocodecpar->codec_id);

    if (!codec) {
      DVMESSAGE("audio encoder \"%s\" not available",
          avcodec_get_name(ocodecpar->codec_id));
      goto e_codec;
    }
  }
  else {
    // we're going to decode and receive needed information from the
    // correponding stream's codec parameters.
    codec=avcodec_find_decoder(codecpar->codec_id);

    if (!codec) {
      DVMESSAGE("audio decoder \"%s\" not available",
          avcodec_get_name(codecpar->codec_id));
      goto e_codec;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  audio->ctx=avcodec_alloc_context3(codec);

  if (!audio->ctx) {
    DMESSAGE("allocating codec context");
    goto e_context;
  }

  ////////////////////////////////////////////////////////////////////////////
  if (ocodecpar) {
    // we're going to encode.
    if (codec->sample_fmts) {
      audio->ctx->sample_fmt=AV_SAMPLE_FMT_NONE;

      // try finding an exact match.
      for (i=0;0<=codec->sample_fmts[i];++i) {
        if (codec->sample_fmts[i]==ocodecpar->format) {
          audio->ctx->sample_fmt=ocodecpar->format;
          break;
        }
      }

#if 0 // [
      if (AV_SAMPLE_FMT_NONE==audio->ctx->sample_fmt) {
        // try swapping planar <--> interleaved.
        for (i=0;0<=codec->sample_fmts[i];++i) {
          switch (codec->sample_fmts[i]) {
          // interleaved //////////////////////////////////////////////////////
          case AV_SAMPLE_FMT_S16:
            if (ocodecpar->format==AV_SAMPLE_FMT_S16P) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_S16;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_S32:
            if (ocodecpar->format==AV_SAMPLE_FMT_S32P) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_S32;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_FLT:
            if (ocodecpar->format==AV_SAMPLE_FMT_FLTP) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_FLT;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_DBL:
            if (ocodecpar->format==AV_SAMPLE_FMT_DBLP) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_DBL;
              goto loop_inner_break;
            }

            continue;
          // planar ///////////////////////////////////////////////////////////
          case AV_SAMPLE_FMT_S16P:
            if (ocodecpar->format==AV_SAMPLE_FMT_S16) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_S16P;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_S32P:
            if (ocodecpar->format==AV_SAMPLE_FMT_S32) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_S32P;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_FLTP:
            if (ocodecpar->format==AV_SAMPLE_FMT_FLT) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_FLTP;
              goto loop_inner_break;
            }

            continue;
          case AV_SAMPLE_FMT_DBLP:
            if (ocodecpar->format==AV_SAMPLE_FMT_DBL) {
              audio->ctx->sample_fmt=AV_SAMPLE_FMT_DBLP;
              goto loop_inner_break;
            }

            continue;
          /////////////////////////////////////////////////////////////////////
          default:
            continue;
          }
        }

        loop_inner_break: ;
      }
#endif // ]

      if (AV_SAMPLE_FMT_NONE==audio->ctx->sample_fmt) {
        // fallback to the default.
        audio->ctx->sample_fmt=codec->sample_fmts[0];
      }
    }
    else
      audio->ctx->sample_fmt=ocodecpar->format;

    if (codec->supported_samplerates) {
      audio->ctx->sample_rate=codec->supported_samplerates[0];

      for (i=0;codec->supported_samplerates[i];++i) {
        if (codec->supported_samplerates[i]==ocodecpar->sample_rate) {
          audio->ctx->sample_rate=ocodecpar->sample_rate;
          break;
        }
      }
    }
    else
      audio->ctx->sample_rate=ocodecpar->sample_rate;

    if (codec->channel_layouts) {
      audio->ctx->channel_layout=codec->channel_layouts[0];

      for (i=0;codec->channel_layouts[i];++i) {
        if (codec->channel_layouts[i]==ocodecpar->channel_layout) {
          audio->ctx->channel_layout=ocodecpar->channel_layout;
          break;
        }
      }
    }
    else
      audio->ctx->channel_layout=ocodecpar->channel_layout;

    audio->ctx->channels
        =av_get_channel_layout_nb_channels(audio->ctx->channel_layout);
    audio->ctx->time_base.num=1;
    audio->ctx->time_base.den=audio->ctx->sample_rate;
#if defined (FF_OUTPUT_BIT_RATE) // [
    audio->ctx->bit_rate=FF_OUTPUT_BIT_RATE;
#endif // ]
    stream->time_base=audio->ctx->time_base;
  }
  else if (iparam) {
    // we're going to decode.
    err=avcodec_parameters_to_context(audio->ctx,codecpar);

    if (err<0) {
      DVMESSAGE("copying codec parameters: %s (%d)",av_err2str(err),err);
      goto e_copy1;
    }

    if (0ll<=iparam->request.channel_layout)
      audio->ctx->request_channel_layout=iparam->request.channel_layout;

		if (0<=iparam->request.sample_fmt)
    	audio->ctx->request_sample_fmt=iparam->request.sample_fmt;

    ///////////////////////////////////////////////////////////////////////////
    if (iparam->drc.enabled&&AV_CODEC_ID_AC3==codecpar->codec_id) {
      sprintf(value,"%0.1f",iparam->drc.scale);
      err=av_dict_set(&opt,"drc_scale",value,0);

      if (err<0) {
        DVMESSAGE("setting drc_scale: %s (%d)",av_err2str(err),err);
        goto e_copy1;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=avcodec_open2(audio->ctx,codec,&opt);
  av_dict_free(&opt);

  if (err<0) {
    DVMESSAGE("copening codec context: %s (%d)",av_err2str(err),err);
    goto e_open;
  }

  if (iparam&&!audio->ctx->channel_layout) {
#if 0 // [
    // this is some strange behaviour: when we request AV_CH_LAYOUT_STEREO
    // from AV_CH_LAYOUT_5POINT1 it might turn out that everything is fine
    // (especially "channels") except "channel_layout" might not be set.
    audio->ctx->channel_layout=iparam->request.channel_layout;
#else // ] [
    if (!audio->ctx->channels) {
    	DMESSAGE("missing input #channels");
    	goto e_channels;
		}

    audio->ctx->channel_layout
        =av_get_default_channel_layout(audio->ctx->channels);
#endif // ]
  }

#if 0 // [
  if (codecpar) {
    // we're going to encode and as promised above we copy the codec
    // context's parameters to the corresponding stream.
#else // ] [
    // we copy the codec parameters anyway because e.g. "sample_fmt" might
    // have changed in case of decoding.
#endif // ]
    err=avcodec_parameters_from_context(codecpar,audio->ctx);

    if (err<0) {
      DVMESSAGE("copying codec parameters: %s (%d)",av_err2str(err),err);
      goto e_copy2;
    }
#if 0 // [
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_copy2:
e_channels:
e_open:
e_copy1:
  avcodec_free_context(&audio->ctx);
e_context:
e_codec:
e_args:
  return -1;
}

void ff_audio_destroy(ff_audio_t *audio)
{
  avcodec_free_context(&audio->ctx);
}
