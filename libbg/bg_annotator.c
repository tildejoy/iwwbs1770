/*
 * bg_annotator.c
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

#if defined (BG_PURGE) // [
///////////////////////////////////////////////////////////////////////////////
static bg_tree_t *bg_tree_annotation_parent(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;

  return param->argv.cur->purge<tree->depth?tree->parent
    :param->argv.cur->purge==tree->depth?&param->root:NULL;
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int bg_file_annotation_create(bg_tree_t *tree)
{
  const bg_param_t *param=tree->param;
#if defined (BG_PURGE) // [
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
#else // ] [
  bg_tree_t *parent=tree->parent;
#endif // ]
  size_t len1,len2;
  ssize_t size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
  len2=tree->source.basename?FFSTRLEN(tree->source.basename):0u;
  size=(len1?len1+1u:0u)+(len2?len2+1u:0u);
  tree->target.path=tp=malloc(size*sizeof tp[0]);

  if (!tree->target.path) {
    DMESSAGE("allocating output path");
    goto e_path;
  }

  if (parent&&parent->target.path) {
    FFSTRCPY(tp,parent->target.path);
    tp+=len1;

    if (tree->source.basename)
      *tp++=FFPATHSEP;
  }

  tree->target.basename=tp;

  if (tree->source.basename) {
    FFSTRCPY(tp,tree->source.basename);
    tp+=len2;
  }
  else
    *tp=0;

  /////////////////////////////////////////////////////////////////////////////
  len2=FFSTRLEN(param->temp_prefix);
  tree->temp.path=tp=malloc((size+len2)*sizeof tp[0]);

  if (!tree->temp.path) {
    DMESSAGE("allocating temporary path");
    goto e_temp;
  }

  if (parent&&parent->target.path) {
    FFSTRCPY(tp,parent->target.path);
    tp+=len1;

    if (tree->source.basename)
      *tp++=FFPATHSEP;
  }

  tree->temp.basename=tp;
  FFSTRCPY(tp,param->temp_prefix);
  tp+=len2;
  FFSTRCPY(tp,tree->target.basename);

  if (ff_fexists(tree->temp.path)) {
    FFVMESSAGE("file \"%s\" exists; either remove it or define another"
        " prefix for temporary files by means of option --temp-prefix",
        tree->temp.path);
    goto e_temp_exists;
  }

  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
success:
#endif // ]
  return 0;
//cleanup:
e_temp_exists:
  free(tree->temp.path);
e_temp:
  free(tree->target.path);
e_path:
  return -1;
}

void bg_file_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
  free(tree->temp.path);
  free(tree->target.path);
}

///////////////////////////////////////////////////////////////////////////////
#define BG_TRACK_AUDIO_SFX
#if defined (BG_TRACK_AUDIO_SFX) // [
static const ffchar_t *bg_track_audio_sfx(const bg_track_t *track FFUNUSED,
    const bg_tree_t *tree, const ffchar_t *default_sfx)
{
  const ffchar_t *sp,*sfx;

  // get the suffix from the input.
  sfx=sp=FFSTRSTR(tree->source.path,FFL("."));

  // find the right-most dot '.'.
  while (sp) {
    sfx=++sp;
    sp=FFSTRSTR(sp,FFL("."));
  }

  if (!sfx)
    return default_sfx;
  else if (track->input.audio.ctx) {
    if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else
      return default_sfx;
  }
  else {
    if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("wv"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("mp3"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("ogg"),sfx))
      return sfx;
    else
      return default_sfx;
  }
}
#endif // ]

#define BG_ALBUM_ART
static const ffchar_t *bg_track_out_sfx(const bg_track_t *track,
    const bg_tree_t *tree)
{
  const bg_param_t *param=tree->param;
#if defined (BG_ALBUM_ART) // [
  AVStream *vstream FFUNUSED=0<track->input.vi?
      track->input.fmt.ctx->streams[track->input.vi]:NULL;
#endif // ]

#if defined (BG_TRACK_AUDIO_SFX) // [
  if (param->out.sfx&&*param->out.sfx) {
    // in case a sfx is provided choose it.
    return param->out.sfx;
  }
  else {
    return bg_track_audio_sfx(track,tree,
        0<=track->input.vi?FFL("mkv"):FFL("mka"));
  }
#else // ] [
  if ((BG_FLAGS_MODE_APPLY&param->flags.mode)
      &&param->out.sfx&&*param->out.sfx) {
    // in case a sfx is provided choose it.
    return param->out.sfx;
  }
#if defined (BG_ALBUM_ART) // [
  else if (vstream&&!(vstream->disposition&AV_DISPOSITION_ATTACHED_PIC)) {
#else // ] [
  else if (0<=track->input.vi) {
#endif // ]
    // in case of a video:
    if (track->input.audio.ctx) {
      // when transcoding we choose MKV as the container.
#if defined (BG_TRACK_AUDIO_SFX) // [
      return bg_track_audio_sfx(track,tree,FFL("mkv"));
#else // ] [
      return FFL("mkv");
#endif // ]
    }
    else {
      // we always remux to mkv.
      return FFL("mkv");
    }
#if ! defined (BG_ALBUM_ART) // [
  }
#else // ] [
  }
#endif // ]
  else if (track->input.audio.ctx) {
    // we always transcode to FLAC.
    return FFL("flac");
	}
  else {
#if defined (BG_TRACK_AUDIO_SFX) // [
    return bg_track_audio_sfx(track,tree,FFL("mka"));
#else // ] [
    // get the suffix from the input.
    sfx=sp=FFSTRSTR(tree->source.path,FFL("."));

    // find the right-most dot '.'.
    while (sp) {
      sfx=++sp;
      sp=FFSTRSTR(sp,FFL("."));
    }

    if (!sfx)
      return FFL("mka");
    else if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("wv"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("mp3"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("ogg"),sfx))
      return sfx;
    else
      return FFL("mka");
#endif // ]
  }
#endif // ]
}

#define BG_TRACK_BASENAME_TRACK
static int bg_track_basename(bg_track_t *track, bg_tree_t *tree,
    ffchar_t **opath, size_t len)
{
#if defined (_WIN32) && defined (BG_TRACK_BASENAME_TRACK) // [
  enum { TRACK_SIZE=64 };
#endif // ]
  bg_param_t *param=tree->param;
  AVDictionary *metadata=track->input.fmt.ctx->metadata;

  struct {
    const AVDictionaryEntry *title;
#if defined (BG_TRACK_BASENAME_TRACK) // [
    const AVDictionaryEntry *track;
#endif // ]
  } tag={
    .title=av_dict_get(metadata,"TITLE",NULL,AV_DICT_IGNORE_SUFFIX),
#if defined (BG_TRACK_BASENAME_TRACK) // [
    .track=av_dict_get(metadata,"TRACK",NULL,AV_DICT_IGNORE_SUFFIX),
#endif // ]
  };

#if defined (_WIN32) && ! defined (BG_WIN32_CREATE_LOCALE) // [
  const char *locale=NULL;
#endif // ]
  // NOTE: under Linux sizeof(wchar_t) is 4 bytes and under Windows
  //   it is 2 bytes.
  wchar_t *op;
  const ffchar_t *ipath,*sfx,*sp;

  if ((BG_FLAGS_EXT_RENAME&param->flags.extension)&&tag.title) {
#if defined (_WIN32) // [
    if (opath) {
      // swprintf() in any case writes a trailing '\0', hence (1+len).
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (tag.track) {
        len=swprintf(*opath,1+track->target.pfx.len,L"%02d_",
            atoi(tag.track->value));
      }
      else
        len=0;
#else // ] [
      swprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      wcscpy(*opath+track->target.pfx.len,track->target.title);
      free(track->target.title);
      track->target.title=NULL;
    }
    else {
      // claculate the length of the intermediate representation.
      len=MultiByteToWideChar(
        CP_UTF8,        // UINT                              CodePage,
        0ul,            // DWORD                             dwFlags,
        tag.title->value,
                        // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
        strlen(tag.title->value),
                        // int                               cbMultiByte,
        NULL,           // LPWSTR                            lpWideCharStr,
        0               // int                               cchWideChar
      );

      // allocate sufficient memory to hold the intermediate representatiion.
      track->target.title=malloc((len+1)*sizeof *track->target.title);

      if (!track->target.title) {
        DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      // transform into the intermediate representation.
      MultiByteToWideChar(
        CP_UTF8,        // UINT                              CodePage,
        0ul,            // DWORD                             dwFlags,
        tag.title->value,
                        // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
        strlen(tag.title->value),
                        // int                               cbMultiByte,
        track->target.title,
                        // LPWSTR                            lpWideCharStr,
        len             // int                               cchWideChar
      );

      // add a trailing '\0' to the intermediate representation.
      track->target.title[len]=L'\0';
      // do some transformations, i.e. replace some characters by '_'
      // and transform to lower case.
      op=track->target.title;
#if ! defined (BG_WIN32_CREATE_LOCALE) // [
      // we need to temporarily switch locale in order that towlower() works
      // as expected because unfortunately on our system _create_locale()
      // isn't availabe and hence we have to rely on towlower().
      locale=setlocale(LC_ALL,NULL);
      setlocale(LC_ALL,"");
#endif // ]

      while (*op) {
        switch (*op) {
        case L'.':
        case L'/':
        case L'\\':
        case L'(':
        case L')':
        case L'&':
        case L':':
        case L' ':
        case L'\'':
          if (track->target.title<op&&'_'==op[-1])
            memmove(op,op+1,(wcslen(op+1)+1)*sizeof *op);
          else {
            *op=L'_';
            op=CharNextW(op);
          }

          break;
        default:
#if defined (BG_WIN32_CREATE_LOCALE) // [
          *op=_towlower_l(*op,tree->param->locale);
#else // ] [
          // our system _create_locale() isn't availabe.
          *op=towlower(*op);
#endif // ]
          op=CharNextW(op);
          break;
        }
      }

      // if necessary shorten the intermediate representation.
      if (track->target.title<op&&'_'==op[-1])
        op[-1]=L'\0';

#if ! defined (BG_WIN32_CREATE_LOCALE) // [
      // we need to switch back locale.
      setlocale(LC_ALL,locale);
#endif // ]

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (tag.track) {
        len=swprintf(NULL,0,L"%02d_",atoi(tag.track->value));
      }
      else
        len=0;
#else // ] [
      len=swprintf(NULL,0,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      len+=wcslen(track->target.title);
    }
#else // ] [
    if (opath) {
      // snprintf() in any case writes a trailing '\0', hence (1+len).
      // I.e. we assume enough memory is allocated for holding a
      // trailing '\0'.
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (tag.track) {
        snprintf(*opath,1+track->target.pfx.len,"%02d_",
            atoi(tag.track->value));
      }
#else // ] [
      snprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      // we convert the intermediate representation back into utf-8.
      wcstombs(*opath+track->target.pfx.len,track->target.title,
          len-track->target.pfx.len);
      free(track->target.title);
      track->target.title=NULL;
    }
    else {
      // calculate the length for holding the intermediate representation.
      len=mbstowcs(NULL,tag.title->value,0);
      // allocate sufficient memory  in order to hold the intermediate
      // representation.
      track->target.title=malloc((len+1)*sizeof track->target.title[0]);

      if (!track->target.title) {
        DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      // convert the title tag into the intermediate representation
      // including a trailing '\0'.
      mbstowcs(track->target.title,tag.title->value,len+1);

      // do some transformations.
      op=track->target.title;

      while (*op) {
        switch (*op) {
        case L'.':
        case L'/':
        case L'\\':
        case L'(':
        case L')':
        case L'&':
        case L':':
        case L' ':
        case L'\'':
          if (track->target.title<op&&'_'==op[-1])
            memmove(op,op+1,(wcslen(op+1)+1)*sizeof *op);
          else {
            *op='_';
            ++op;
          }

          break;
        default:
          *op=towlower(*op);
          ++op;
          break;
        }
      }

      if (track->target.title<op&&'_'==op[-1])
        op[-1]=L'\0';

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (tag.track)
        len=snprintf(NULL,0,"%02d_",atoi(tag.track->value));
      else
        len=0;
#else // ] [
      len=snprintf(NULL,0,track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      // add the length for holding the title (transformed to lower case.)
      len+=wcstombs(NULL,track->target.title,0);
    }
#endif // ]
  }
  else {
    if (tree->source.basename) {
      ipath=tree->source.basename;

      if (opath)
        memcpy(*opath,ipath,len*sizeof **opath);
      else {
        sfx=NULL;
        sp=FFSTRSTR(ipath,FFL("."));

        while (sp) {
          sfx=sp;
          sp=FFSTRSTR(++sp,FFL("."));
        }

        len=sfx?(size_t)(sfx-ipath):FFSTRLEN(ipath);
      }
    }
    else
      len=0;
  }
e_path:
  return len;
}

///////////////////////////////////////////////////////////////////////////////
int bg_track_annotation_create(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;
#if defined (BG_PURGE) // [
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
#else // ] [
  bg_tree_t *parent=tree->parent;
#endif // ]
  size_t len1,len2;
  // before determining the suffix we need to have the file re-opened.
  const ffchar_t *sfx;
  size_t len3;
  size_t size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  if (param->overwrite) {
    // including the path separator!
#if 0 // [
    // pointer arithmetics takes into account the element's size!
    len1/=sizeof tree->source.path[0];
#else // ] [
    len1=tree->source.basename-tree->source.path;
#endif // ]
    len2=bg_track_basename(track,tree,NULL,0);
    // before determining the suffix we need to have the file re-opened.
    sfx=bg_track_out_sfx(track,tree);
    len3=sfx?FFSTRLEN(sfx):0u;
    size=len1+(len2?len2+1u:0u)+(len3?len3+2u:0u);
    tree->target.path=tp=malloc(size*sizeof tp[0]);

    if (!tree->target.path) {
      DMESSAGE("allocating output path");
      goto e_path;
    }

    // we copy the path separator!
    memcpy(tp,tree->source.path,len1*sizeof tree->source.path[0]);
    tp+=len1;

    tree->target.basename=tp;
    bg_track_basename(track,tree,&tp,len2);
    tp+=len2;
    *tp=0;

    if (sfx) {
      *tp++=FFL('.');
      FFSTRCPY(tp,sfx);
      tp+=len3;
    }

    *tp=0;
  }
  else {
    len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
    len2=bg_track_basename(track,tree,NULL,0);
    // before determining the suffix we need to have the file re-opened.
    sfx=bg_track_out_sfx(track,tree);
    len3=sfx?FFSTRLEN(sfx):0u;
    size=(len1?len1+1u:0u)+(len2?len2+1u:0u)+(len3?len3+2u:0u);
    tree->target.path=tp=malloc(size*sizeof tp[0]);

    if (!tree->target.path) {
      DMESSAGE("allocating output path");
      goto e_path;
    }

    if (parent&&parent->target.path) {
      FFSTRCPY(tp,parent->target.path);
      tp+=len1;
      *tp++=FFPATHSEP;
    }

    tree->target.basename=tp;
    bg_track_basename(track,tree,&tp,len2);
    tp+=len2;
    *tp=0;

    if (sfx) {
      *tp++=FFL('.');
      FFSTRCPY(tp,sfx);
      tp+=len3;
    }

    *tp=0;
  }

  if (!param->overwrite&&!ff_fcmp(tree->source.path,tree->target.path)) {
    DMESSAGE("overwriting not permitted. Use option --overwrite");
    goto e_overwrite_target;
  }

  /////////////////////////////////////////////////////////////////////////////
  len2=FFSTRLEN(param->temp_prefix);
  tree->temp.path=tp=malloc((size+len2)*sizeof tp[0]);

  if (!tree->temp.path) {
    DMESSAGE("allocating temporary path");
    goto e_temp;
  }

  memcpy(tp,tree->target.path,len1*sizeof tp[0]);
  tp+=len1;

  if (FFPATHSEP!=tp[-1])
    *tp++=FFPATHSEP;

  tree->temp.basename=tp;
  FFSTRCPY(tp,param->temp_prefix);
  tp+=len2;
  FFSTRCPY(tp,tree->target.basename);

  /////////////////////////////////////////////////////////////////////////////
  if (!ff_fcmp(tree->source.path,tree->temp.path)) {
    DMESSAGE("attempt to overwrite source file");
    goto e_overwrite_temp;
  }

  if (ff_fexists(tree->temp.path)) {
    FFVMESSAGE("file \"%s\" exists; either remove it or by means of"
        " option --temp-prefix define a prefix for temporary files different"
        " from \"%s\"",
        tree->temp.path,param->temp_prefix);
    goto e_temp_exists;
  }

#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // the basename.
  if (tree->param->oem&&tree->target.basename) {
    ///////////////////////////////////////////////////////////////////////////
    track->target.oem.basename=bg_wcs2str(tree->target.basename,CP_OEMCP);

    if (!track->target.oem.basename) {
      DMESSAGE("creating oem basename");
      goto e_basename;
    }
  }
  else
    track->target.oem.basename=NULL;

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_WIN32_TARGET_UTF8) // [
  track->target.utf8.path=bg_wcs2str(tree->target.path,CP_UTF8);
  
  if (!track->target.utf8.path) {
    DMESSAGE("creating utf-8 representation of path");
    goto e_patha;
  }

  track->target.utf8.basename=bg_basenamea(track->target.utf8.path);
#else // ] [
  track->target.path=bg_wcs2str(tree->target.path,CP_UTF8);
  
  if (!track->target.path) {
    DMESSAGE("creating utf-8 representation of path");
    goto e_patha;
  }

  track->target.basename=bg_basenamea(track->target.path);
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  track->temp.path=bg_wcs2str(tree->temp.path,CP_UTF8);
  
  if (!track->temp.path) {
    DMESSAGE("creating utf-8 representation of temporary path");
    goto e_tempa;
  }
#endif // ]

  return 0;
//cleanup:
#if defined (_WIN32) // [
  free(track->temp.path);
e_tempa:
#if defined (BG_WIN32_TARGET_UTF8) // [
  free(track->target.utf8.path);
#else // ] [
  free(track->target.path);
#endif // ]
e_patha:
  if (track->target.oem.basename)
    free(track->target.oem.basename);
e_basename:
#endif // ]
e_temp_exists:
e_overwrite_temp:
  free(tree->temp.path);
e_temp:
e_overwrite_target:
  free(tree->target.path);
e_path:
  return -1;
}

void bg_track_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
#if defined (_WIN32) // [
  bg_track_t *track=&tree->track;

  free(track->temp.path);
#if defined (BG_WIN32_TARGET_UTF8) // [
  free(track->target.utf8.path);
#else // ] [
  free(track->target.path);
#endif // ]
  if (track->target.oem.basename)
    free(track->target.oem.basename);
#endif // ]
  free(tree->temp.path);
  free(tree->target.path);
}

///////////////////////////////////////////////////////////////////////////////
int bg_album_annotation_create(bg_tree_t *tree)
{
#if defined (BG_PURGE) // [
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
  const ffchar_t *basename=tree->source.basename;
#else // ] [
  bg_tree_t *parent=tree->parent;
  const ffchar_t *basename;
#endif // ]
  size_t len1,len2,size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  if (!parent)
    goto success;

  if (tree->param->output.dirname) {
    ///////////////////////////////////////////////////////////////////////////
#if ! defined (BG_PURGE) // [
    basename=bg_album_target_purge(tree);
#endif // ]
    len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
    len2=basename?FFSTRLEN(basename):0u;
    size=(len1?len1+1u:0u)+(len2?len2+1u:0u);

    ///////////////////////////////////////////////////////////////////////////
    if (0u<size) {
      /////////////////////////////////////////////////////////////////////////
      tree->target.path=tp=malloc(size*sizeof tp[0]);

      if (!tree->target.path) {
        DMESSAGE("allocating output path");
        goto e_path;
      }

      if (parent&&parent->target.path) {
        FFSTRCPY(tp,parent->target.path);
        tp+=len1;

        if (basename)
          *tp++=FFPATHSEP;
        else
          *tp=FFL('\0');
      }

      if (basename) {
        tree->target.basename=tp;
        FFSTRCPY(tp,basename);
        tp+=len2;
      }
      else
        tree->target.basename=NULL;

      /////////////////////////////////////////////////////////////////////////
      if (!tree->param->overwrite&&ff_mkdir(tree->target.path)<0) {
  	    DMESSAGE("creating directory");
  	    goto e_mkdir;
      }
    }
    else {
      tree->target.basename=NULL;
      tree->target.path=NULL;
    }
  }
  else {
    tree->target.basename=NULL;
    tree->target.path=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
success:
  return 0;
//cleanup:
#if 1 // [
e_mkdir:
#endif // ]
  if (tree->target.path)
    free(tree->target.path);
e_path:
  return -1;
}

void bg_album_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
  if (tree->target.path)
    free(tree->target.path);
}


///////////////////////////////////////////////////////////////////////////////
int bg_root_annotation_create(bg_tree_t *tree)
{
  if (tree->param->output.dirname) {
    ///////////////////////////////////////////////////////////////////////////
    tree->target.path=tree->param->output.dirname;

    if (!tree->param->overwrite&&ff_mkdir(tree->target.path)<0) {
      /////////////////////////////////////////////////////////////////////////
  	  DMESSAGE("creating directory");
      goto e_mkdir;
    }

    tree->target.basename=NULL;
  }
  else {
    tree->target.path=NULL;
    tree->target.basename=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
// cleanup:
e_mkdir:
  return -1;
}

void bg_root_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
}
