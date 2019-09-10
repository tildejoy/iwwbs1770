/*
 * bg.h
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
#if ! defined (__BG_H__) // [
#define __BG_H__
#if defined (HAVE_CONFIG_H) // [
#include <config.h>
#endif // ]
#include <stdio.h>
#include <pbutil_priv.h>
#include <ff.h>
#include <lib1770.h>
#include <bg_pilot.h>

#if defined (__cplusplus) // [
extern "C" {
#endif // ]

///////////////////////////////////////////////////////////////////////////////
//#define BG_LIST
#define BG_CHANNEL_LFE 3
#define BG_TEMP_PREFIX FFL(".")
#define BG_PURGE
#if defined (BG_PURGE) // [
#define BG_PURGEEX
#endif // ]
//#define BG_PARAM_SLEEP
//#define BG_BWF_TAGS
//#define BG_TREE_CREATE_CHILD_WARNING
#if defined (_WIN32) // [
#define BG_WIN32_TARGET_UTF8
// WARNING:  define BG_WIN32_CREATE_LOCALE at your own risk: at the run-time
//    of our system _create_locale() doesn't exist and hence you're compiling
//    un-tested code.
//#define BG_WIN32_CREATE_LOCALE
#endif // ]

///////////////////////////////////////////////////////////////////////////////
typedef unsigned long bg_bits_t;
typedef struct bg_album bg_album_t;
typedef enum bg_track_tag bg_track_tag_t;
typedef struct bg_track_target bg_track_target_t;
typedef struct bg_track bg_track_t;
typedef enum bg_tree_type bg_tree_type_t;
typedef const struct bg_annotation_vmt bg_annotation_vmt_t;
typedef const struct bg_tree_vmt bg_tree_vmt_t;
typedef struct bg_tree_path bg_tree_path_t;
#if defined (_WIN32) // [
typedef struct bg_tree_patha bg_tree_patha_t;
#endif // ]
typedef struct bg_tree bg_tree_t;

typedef const struct bg_visitor_vmt bg_visitor_vmt_t;
typedef struct bg_visitor bg_visitor_t;

typedef struct bg_param_block bg_param_block_t;
typedef enum bg_flags_agg bg_flags_agg_t;
typedef enum bg_flags_ext bg_flags_ext_t;
typedef enum bg_flags_mode bg_flags_mode_t;
typedef enum bg_flags_norm bg_flags_norm_t;
typedef const struct bg_print_vmt bg_print_vmt_t;
typedef const struct bg_print_conf bg_print_conf_t;
typedef const struct bg_param_unit bg_param_unit_t;
typedef struct bg_param bg_param_t;
#if defined (BG_PURGE) // [
typedef struct bg_param_argv bg_param_argv_t;
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int64_t bg_parse_time(const ffchar_t *s);
ffchar_t *bg_pathnorm(ffchar_t *path);
const ffchar_t *bg_basename(const ffchar_t *path);
#if defined (_WIN32) // [
char *bg_wcs2str(const wchar_t *wcs, unsigned int codepage);
const char *bg_basenamea(const char *path);
#endif // ]

///////////////////////////////////////////////////////////////////////////////
enum bg_tree_type {
  BG_TREE_TYPE_NULL=0,
  BG_TREE_TYPE_FILE=1<<0,
  BG_TREE_TYPE_TRACK=1<<1,
  BG_TREE_TYPE_ALBUM=1<<2,
  BG_TREE_TYPE_ROOT=1<<3,
  BG_TREE_TYPE_LEAF=BG_TREE_TYPE_FILE|BG_TREE_TYPE_TRACK,
  BG_TREE_TYPE_BRANCH=BG_TREE_TYPE_ALBUM|BG_TREE_TYPE_ROOT,
};

struct bg_annotation_vmt {
  int (*create)(bg_tree_t *tree);
  void (*destroy)(bg_tree_t *tree);
};

struct bg_tree_vmt {
#if defined (PBU_DEBUG) // [
  const ffchar_t *id;
#endif // ]
  const bg_tree_type_t type;
  void (*destroy)(bg_tree_t *tree);
  int (*accept)(bg_tree_t *tree, bg_visitor_t *vis);
  bg_annotation_vmt_t annotation;
};

////////
int bg_root_create(bg_tree_t *tree, bg_param_t *param);

int bg_root_annotation_create(bg_tree_t *tree);
void bg_root_annotation_destroy(bg_tree_t *tree);

////////
struct bg_album {
  // counts BG_TREE_TYPE_FILEs and BG_TREE_TYPE_TRACKs.
#if defined (BG_PURGE) // [
  unsigned nchildren;
#endif // ]
  unsigned nleafs;
  bg_tree_t *first;
  bg_tree_t *last;
};

int bg_album_create(bg_tree_t **tree, bg_param_t *param, bg_tree_t *parent,
    const ffchar_t *path);
void bg_album_destroy(bg_tree_t *tree);

int bg_album_content_create(bg_tree_t *tree, bg_tree_vmt_t *vmt);
void bg_album_content_destroy(bg_tree_t *tree);
int bg_album_push(bg_tree_t *tree, bg_tree_t *child);
bg_tree_t *bg_album_pop(bg_tree_t *tree);

int bg_album_annotation_create(bg_tree_t *tree);
void bg_album_annotation_destroy(bg_tree_t *tree);

#if ! defined (BG_PURGE) // [
const ffchar_t *bg_album_target_purge(bg_tree_t *tree);
#endif // ]

////////
enum bg_track_tag {
  BG_TRACK_TAG_ALGORITHM=1<<0,
  BG_TRACK_TAG_REFERENCE_LOUDNESS=1<<1,
  BG_TRACK_TAG_TRACK_GAIN=1<<2,
  BG_TRACK_TAG_TRACK_PEAK=1<<3,
  BG_TRACK_TAG_ALBUM_GAIN=1<<4,
  BG_TRACK_TAG_ALBUM_PEAK=1<<5,
  BG_TRACK_TAG_MAX=1<<6,
};

////////
struct bg_track_target {
#if defined (_WIN32) // [
#if defined (BG_WIN32_TARGET_UTF8) // [
  struct {
#endif // ]
    char *path;
    const char *basename;
#if defined (BG_WIN32_TARGET_UTF8) // [
  } utf8;
#endif // ]

  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // e.g. basename.
  struct {
    char *basename;
  } oem;
#endif // ]
  wchar_t *title;

  struct {
    size_t len;
  } pfx;
};

////////
struct bg_track {
  ff_inout_t input;
  ff_analyzer_t analyzer;
  bg_track_target_t target;

  // an utf-8 representation of the target path.
  struct {
    char *path;
  } temp;

  struct {
    unsigned long id;
  } root;

  struct {
    unsigned long id;
  } album;

  struct {
    lib1770_pre_t *pre;
  } filter;

  struct {
    lib1770_block_t *momentary;
    lib1770_block_t *shortterm;
  } block;
};

int bg_track_content_create(bg_tree_t *tree);

int bg_track_annotation_create(bg_tree_t *tree);
void bg_track_annotation_destroy(bg_tree_t *tree);

int bg_file_content_create(bg_tree_t *tree);

int bg_file_annotation_create(bg_tree_t *tree);
void bg_file_annotation_destroy(bg_tree_t *tree);

////////
struct bg_tree_path {
  ffchar_t *path;
  const ffchar_t *basename;
};

int bg_tree_source_create(bg_tree_path_t *tp, const ffchar_t *path);
void bg_tree_path_destroy(bg_tree_path_t *tp);

#if defined (_WIN32) // [
////////
struct bg_tree_patha {
  char *path;
  const char *basename;
};

int bg_tree_patha_create(bg_tree_patha_t *p, const wchar_t *path,
    unsigned int codepage);
void bg_tree_patha_destroy(bg_tree_patha_t *p);
#endif // ]

////////
struct bg_tree {
  bg_tree_vmt_t *vmt;
  bg_param_t *param;
  bg_tree_path_t source;

#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // e.g. basename.
  struct {
    char *basename;
  } oem;

  bg_tree_patha_t utf8;
#endif // ]

  bg_tree_path_t target;
  bg_tree_path_t temp;
  bg_tree_t *parent;
#if defined (BG_PURGE) // [
  unsigned depth;
#endif // ]
  bg_tree_t *next;
  bg_tree_t *prev;

  struct {
    lib1770_stats_t *momentary;
    lib1770_stats_t *shortterm;
    double samplepeak;
    double truepeak;
  } stats;

  union {
    bg_album_t album;
    bg_track_t track;
  };
};

int bg_tree_common_create(bg_tree_t *tree, bg_param_t *param,
    bg_tree_t *parent, const ffchar_t *path);
void bg_tree_common_destroy(bg_tree_t *tree);
int bg_tree_stats_create(bg_tree_t *tree);
void bg_tree_stats_destroy(bg_tree_t *tree);

// narrow character representation of the basename.
const char *bg_tree_in_basanamen(bg_tree_t *tree);
const char *bg_tree_out_basanamen(bg_tree_t *tree);
#if defined (_WIN32) // [
// wide character representation of the basename.
const wchar_t *bg_tree_in_basanamew(bg_tree_t *tree);
const wchar_t *bg_tree_out_basanamew(bg_tree_t *tree);
#endif // ]

int bg_leaf_create(bg_tree_t **tree, bg_param_t *param, bg_tree_t *parent,
    const ffchar_t *path);

double *bg_tree_samplepeak(bg_tree_t *tree);
double *bg_tree_truepeak(bg_tree_t *tree);
int bg_tree_merge(bg_tree_t *lhs, const bg_tree_t *rhs);

///////////////////////////////////////////////////////////////////////////////
// several phases are implemented by means of the visitor pattern (cf. e.g.
// https://en.wikipedia.org/wiki/Visitor_pattern.) we've got the following
// phases:
// 1) annotation (cf. below),
// 2) analysis, and
// 3) re-muxing/transcoding.
struct bg_visitor_vmt {
#if defined (PBU_DEBUG) // [
  const char *id;
#endif // ]
  void (*destroy)(bg_visitor_t *vis);
  int (*dispatch_file)(bg_visitor_t *vis, bg_tree_t *tree);
  int (*dispatch_track)(bg_visitor_t *vis, bg_tree_t *tree);
  int (*dispatch_album)(bg_visitor_t *vis, bg_tree_t *tree);
  int (*dispatch_root)(bg_visitor_t *vis, bg_tree_t *tree);
};

int bg_analyzer_create(bg_visitor_t *vis);
int bg_muxer_create(bg_visitor_t *vis);

int bg_analyzer_album_prefix(bg_visitor_t *vis, bg_tree_t *tree);
int bg_analyzer_album_suffix(bg_visitor_t *vis, bg_tree_t *tree);

////////
struct bg_visitor {
  bg_visitor_vmt_t *vmt;
  int depth;
};

///////////////////////////////////////////////////////////////////////////////
struct bg_print_vmt {
  const char *id;
  int infix;
  void (*encoding)(bg_param_t *param, int bits);
  int (*head)(bg_tree_t *tree, int depth, FILE *f);
  int (*tail)(bg_tree_t *tree, int depth, FILE *f);
};

extern bg_print_vmt_t bg_print_classic_vmt;
extern bg_print_vmt_t bg_print_xml_vmt;

////////
struct bg_param_block {
  double ms;
  int partition;

  struct {
    double gate;
  } mean;

  struct {
    double gate;
    double lower_bound;
    double upper_bound;
  } range;
};

////////
enum {
  BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET,
  BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET,
  BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET,
  BG_FLAGS_AGG_SAMPLEPEAK_OFFSET,
  BG_FLAGS_AGG_TRUEPEAK_OFFSET,
  BG_FLAGS_AGG_MAX_OFFSET,
};

enum bg_flags_agg {
  BG_FLAGS_AGG_MOMENTARY_MEAN=1<<BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET,
  BG_FLAGS_AGG_MOMENTARY_MAXIMUM=1<<BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET,
  BG_FLAGS_AGG_MOMENTARY_RANGE=1<<BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_MEAN=1<<BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_MAXIMUM=1<<BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET,
  BG_FLAGS_AGG_SHORTTERM_RANGE=1<<BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET,
  BG_FLAGS_AGG_SAMPLEPEAK=1<<BG_FLAGS_AGG_SAMPLEPEAK_OFFSET,
  BG_FLAGS_AGG_TRUEPEAK=1<<BG_FLAGS_AGG_TRUEPEAK_OFFSET,
  BG_FLAGS_AGG_MAX=1<<BG_FLAGS_AGG_MAX_OFFSET,
  BG_FLAGS_AGG_MOMENTARY
      =BG_FLAGS_AGG_MOMENTARY_MAXIMUM
      |BG_FLAGS_AGG_MOMENTARY_MEAN
      |BG_FLAGS_AGG_MOMENTARY_RANGE,
  BG_FLAGS_AGG_SHORTTERM
      =BG_FLAGS_AGG_SHORTTERM_MAXIMUM
      |BG_FLAGS_AGG_SHORTTERM_MEAN
      |BG_FLAGS_AGG_SHORTTERM_RANGE,
  BG_FLAGS_AGG_PEAK
      =BG_FLAGS_AGG_SAMPLEPEAK
      |BG_FLAGS_AGG_TRUEPEAK,
  BG_FLAGS_AGG_ALL
      =BG_FLAGS_AGG_MOMENTARY
      |BG_FLAGS_AGG_SHORTTERM
      |BG_FLAGS_AGG_SAMPLEPEAK
      |BG_FLAGS_AGG_TRUEPEAK,
  BG_FLAGS_AGG_INTEGRATED=BG_FLAGS_AGG_MOMENTARY_MEAN,
};

////////
enum bg_flags_ext {
  BG_FLAGS_EXT_RENAME=1<<1,
  BG_FLAGS_EXT_CSV=1<<2,
  BG_FLAGS_EXT_COPY=1<<3,
  BG_FLAGS_EXT_TAGS=1<<4,
  BG_FLAGS_EXT_ALL
      =BG_FLAGS_EXT_RENAME
      |BG_FLAGS_EXT_CSV
      |BG_FLAGS_EXT_COPY
      |BG_FLAGS_EXT_TAGS,
};

////////
enum bg_flags_mode {
  BG_FLAGS_MODE_APPLY=1<<0,
#if defined (BG_BWF_TAGS) // [
  BG_FLAGS_MODE_TAGS_RG=1<<1,
  BG_FLAGS_MODE_TAGS_BWF=1<<2,
#endif // ]
  BG_FLAGS_MODE_TAGS_TRACK=1<<3,
  BG_FLAGS_MODE_TAGS_ALBUM=1<<4,
  BG_FLAGS_MODE_TAGS_ALL
      =BG_FLAGS_MODE_TAGS_TRACK
      |BG_FLAGS_MODE_TAGS_ALBUM,
};

////////
enum bg_flags_norm {
  BG_FLAGS_NORM_NULL=0ul,
  BG_FLAGS_NORM_EBU=1ul<<1ul,
  BG_FLAGS_NORM_ATSC=1ul<<2ul,
  BG_FLAGS_NORM_REPLAYGAIN=1ul<<3ul,
};

////////
struct bg_param_unit {
  // narrow string representation.
  struct {
    const char *lu;
    const char *lra;
    const char *sp;
    const char *tp;
  } n;

#if defined (_WIN32) // [
  // wide string representation.
  struct {
    const wchar_t *lu;
    const wchar_t *lra;
    const wchar_t *sp;
    const wchar_t *tp;
  } w;
#endif // ]
};

#if defined (BG_PURGE) // [
////////
struct bg_param_argv {
  unsigned purge;
};
#endif // ]

////////
struct bg_param {
#if defined (BG_PURGE) // [
  struct {
    bg_param_argv_t *min;
    bg_param_argv_t *cur;
    bg_param_argv_t *max;
  } argv;
#endif // ]

  struct count {
    unsigned long cur;
    unsigned long max;
  } count;

  int process;
  bg_pilot_t pilot;
  bg_visitor_t analyzer;
  bg_tree_t root,*tos;

  struct {
    bg_print_vmt_t *vmt;
  } print;

  ff_printer_t printer;
  int loglevel;
  int purged;

  struct {
#if defined (_WIN32) // [
    char name[64];
#else // ] [
    char *name;
#endif // ]
  } codec;

  const ffchar_t *temp_prefix;

  struct {
    int hierarchy;
    int progress;
  } suppress;

#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // e.g. basename.
  int oem;
#endif // ]
  ff_param_decode_t decode;

  struct {
    FILE *f;
  } result;

  struct {
    ffchar_t *dirname;
  } output;

  int ai,vi;

  struct {
    bg_flags_ext_t extension;
    bg_flags_mode_t mode;
    bg_flags_agg_t aggregate;
    bg_flags_norm_t norm;
  } flags;

#if defined (BG_PARAM_DUMP) // [
  int dump;
#endif // ]
#if defined (BG_PARAM_STEREO) // [
  int stereo;
#endif // ]
  double norm;
  double preamp;

  struct {
    int enabled;
    double value;
  } weight;

  int time;
  int lfe;
#if defined (_WIN32) && defined (BG_WIN32_CREATE_LOCALE) // [
  _locale_t locale;
#endif // ]
  int overwrite;
  bg_param_unit_t *unit;

  struct {
    char pfx[128];
  } tag;

  struct {
    ffchar_t *sfx;
  } out;

  struct {
    const ffchar_t *audio;
    const ffchar_t *video;
  } ext;

  struct {
    int64_t begin;
    int64_t duration;
  } interval;

#if defined (BG_PARAM_SLEEP) // [
  int sleep;
#endif // ]
  bg_param_block_t momentary;
  bg_param_block_t shortterm;
};

int bg_param_create(bg_param_t *param);
void bg_param_destroy(bg_param_t *param);

#if defined (BG_PURGE) // [
int bg_param_alloc_arguments(bg_param_t *param, size_t size);
void bg_param_free_argumets(bg_param_t *param);
#endif // ]
void bg_param_set_unit_ebu(bg_param_t *param);
void bg_param_set_unit_db(bg_param_t *param);
void bg_param_set_process(bg_param_t *param);
int bg_param_loop(bg_param_t *param, ffchar_t *const *argv, int i, int argc);

///////////////////////////////////////////////////////////////////////////////
struct bg_print_conf {
  bg_flags_agg_t aggregate;

#if defined (_WIN32) // [
  // when not writing to the console/shell (i.e. to a file) we need to use
  // the wide character representation.
  struct {
    const wchar_t *label;

    struct {
      const wchar_t *classic;
      const wchar_t *xml;
    } format;

    const wchar_t *(*unit)(bg_tree_t *tree);
  } w;
#endif // ]

  // narrow character representation.
  struct {
    const char *label;

    struct {
      const char *classic;
      const char *xml;
    } format;

    const char *(*unit)(bg_tree_t *tree);
  } n;

  int unitc;
  int argc;
  double (*argv[2])(bg_tree_t *tree);
};

extern bg_print_conf_t bg_print_conf[BG_FLAGS_AGG_MAX_OFFSET];

#if defined (__cplusplus) // [
}
#endif // ]
#endif // __BG_H__ ]
