/*
 * bg_tree.c
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
#include <bg.h>

///////////////////////////////////////////////////////////////////////////////
int bg_tree_common_create(bg_tree_t *tree, bg_param_t *param,
    bg_tree_t *parent, const ffchar_t *path)
{
  /////////////////////////////////////////////////////////////////////////////
  memset(tree,0,sizeof *tree);

  /////////////////////////////////////////////////////////////////////////////
  tree->vmt=NULL;
  tree->param=param;
  tree->parent=parent;
#if defined (BG_PURGE) // [
  tree->depth=parent?1u+parent->depth:0u;
#endif // ]
  tree->next=NULL;
  tree->prev=NULL;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_source_create(&tree->source,path)<0) {
    DMESSAGE("creating source path");
    goto e_path;
  }

#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // the basename.
  if (param->oem&&tree->source.basename) {
    tree->oem.basename=bg_wcs2str(tree->source.basename,CP_OEMCP);

    if (!tree->oem.basename) {
      DMESSAGE("creating oem basename");
      goto e_basename;
    }
  }
  else
    tree->oem.basename=NULL;

  // in any case we need an utf-8 representation of path (end basaname.)
  if (bg_tree_patha_create(&tree->utf8,tree->source.path,CP_UTF8)<0) {
    DMESSAGE("creating patha");
    goto e_patha;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_stats_create(tree)<0) {
    DMESSAGE("creating stats");
    goto e_stats;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  bg_tree_stats_destroy(tree);
e_stats:
#if defined (_WIN32) // [
  bg_tree_patha_destroy(&tree->utf8);
e_patha:
  if (tree->oem.basename)
    free(tree->oem.basename);
e_basename:
#endif // ]
  bg_tree_path_destroy(&tree->source);
e_path:
  return -1;
}

void bg_tree_common_destroy(bg_tree_t *tree)
{
  bg_tree_stats_destroy(tree);
#if defined (_WIN32) // [
  bg_tree_patha_destroy(&tree->utf8);

  if (tree->oem.basename)
    free(tree->oem.basename);
#endif // ]

  bg_tree_path_destroy(&tree->source);
}

int bg_tree_stats_create(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;

  if (param->process) {
    ///////////////////////////////////////////////////////////////////////////
    if (BG_FLAGS_AGG_MOMENTARY&param->flags.aggregate) {
      tree->stats.momentary=lib1770_stats_new();

      if (!tree->stats.momentary) {
        DMESSAGE("creating momentary statistics");
        goto emomentary;
      }
    }
    else
      tree->stats.momentary=NULL;

    ///////////////////////////////////////////////////////////////////////////
    if (BG_FLAGS_AGG_SHORTTERM&param->flags.aggregate) {
      tree->stats.shortterm=lib1770_stats_new();

      if (!tree->stats.shortterm) {
        DMESSAGE("creating shortterm statistics");
        goto eshortterm;
      }
    }
    else
      tree->stats.shortterm=NULL;
  }
  else {
    tree->stats.momentary=NULL;
    tree->stats.shortterm=NULL;
  }

  tree->stats.samplepeak=0.0;
  tree->stats.truepeak=0.0;

  return 0;
//cleanup:
  if (tree->stats.shortterm)
    lib1770_stats_close(tree->stats.shortterm);
eshortterm:
  if (tree->stats.momentary)
    lib1770_stats_close(tree->stats.momentary);
emomentary:
  return -1;
}

void bg_tree_stats_destroy(bg_tree_t *tree)
{
  if (tree->stats.shortterm) {
    lib1770_stats_close(tree->stats.shortterm);
    tree->stats.shortterm=NULL;
  }

  if (tree->stats.momentary) {
    lib1770_stats_close(tree->stats.momentary);
    tree->stats.momentary=NULL;
  }
}

int bg_leaf_create(bg_tree_t **tree, bg_param_t *param, bg_tree_t *parent,
    const ffchar_t *path)
{
  /////////////////////////////////////////////////////////////////////////////
  *tree=malloc(sizeof **tree);

  if (!*tree) {
#if defined (BG_TREE_CREATE_CHILD_WARNING) // [
    DWARNING("allocating tree");
#endif // ]
    goto e_malloc;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_common_create(*tree,param,parent,path)<0) {
#if defined (BG_TREE_CREATE_CHILD_WARNING) // [
    DWARNING("creating tree");
#endif // ]
    goto e_common;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (0<=bg_track_content_create(*tree)) {
    ;
  }
  else if (!param->overwrite
      &&(BG_FLAGS_EXT_COPY&param->flags.extension)
      &&0<=bg_file_content_create(*tree)) {
    ;
  }
  else {
#if defined (BG_TREE_CREATE_CHILD_WARNING) // [
    FFVWARNING(FFL("creating leaf \"%s\""),(*tree)->path.source);
#endif // ]
    goto e_child;
  }

  if (parent&&bg_album_push(parent,*tree)<0) {
    DMESSAGE("pushing");
    goto e_push;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (parent)
    bg_album_pop(parent);
e_push:
e_child:
  bg_tree_common_destroy(*tree);
e_common:
  free(*tree);
  *tree=NULL;
e_malloc:
  return -1;
}

double *bg_tree_samplepeak(bg_tree_t *tree)
{
  return tree->param->flags.aggregate&BG_FLAGS_AGG_SAMPLEPEAK
      ?&tree->stats.samplepeak:NULL;
}

double *bg_tree_truepeak(bg_tree_t *tree)
{
  return tree->param->flags.aggregate&BG_FLAGS_AGG_TRUEPEAK
      ?&tree->stats.truepeak:NULL;
}

int bg_tree_merge(bg_tree_t *lhs, const bg_tree_t *rhs)
{
  bg_flags_agg_t aggregate=lhs->param->flags.aggregate;

  if (aggregate!=rhs->param->flags.aggregate) {
    DMESSAGE("trying to merge incompatible trees");
    goto emerge;
  }

  if (BG_TREE_TYPE_FILE==lhs->vmt->type||BG_TREE_TYPE_FILE==rhs->vmt->type)
    goto success;

  if (lhs->param->process) {
#if 1 // [
    if (0!=(BG_FLAGS_AGG_MOMENTARY&aggregate))
      lib1770_stats_merge(lhs->stats.momentary,rhs->stats.momentary);

    if (0!=(BG_FLAGS_AGG_SHORTTERM&aggregate))
      lib1770_stats_merge(lhs->stats.shortterm,rhs->stats.shortterm);

    if (0!=(BG_FLAGS_AGG_SAMPLEPEAK&aggregate)) {
      if (lhs->stats.samplepeak<rhs->stats.samplepeak)
        lhs->stats.samplepeak=rhs->stats.samplepeak;
    }

    if (0!=(BG_FLAGS_AGG_TRUEPEAK&aggregate)) {
      if (lhs->stats.truepeak<rhs->stats.truepeak)
        lhs->stats.truepeak=rhs->stats.truepeak;
    }
#else // ] [
    if (lhs->stats.momentary&&rhs->stats.momentary)
      lib1770_stats_merge(lhs->stats.momentary,rhs->stats.momentary);

    if (lhs->stats.shortterm&&rhs->stats.shortterm)
      lib1770_stats_merge(lhs->stats.shortterm,rhs->stats.shortterm);

    if (0!=(BG_FLAGS_AGG_SAMPLEPEAK&aggregate)) {
      if (lhs->stats.samplepeak<rhs->stats.samplepeak)
        lhs->stats.samplepeak=rhs->stats.samplepeak;
    }

    if (0!=(BG_FLAGS_AGG_TRUEPEAK&aggregate)) {
      if (lhs->stats.truepeak<rhs->stats.truepeak)
        lhs->stats.truepeak=rhs->stats.truepeak;
    }
#endif // ]
  }

success:
  return 0;
emerge:
  return -1;
}

#if defined (_WIN32) // [
const char *bg_tree_in_basanamen(bg_tree_t *tree)
{
  return tree->oem.basename?tree->oem.basename:tree->utf8.basename;
}

const wchar_t *bg_tree_in_basanamew(bg_tree_t *tree)
{
  return tree->source.basename;
}

const char *bg_tree_out_basanamen(bg_tree_t *tree)
{
  bg_track_t *track=&tree->track;

  // just needed for tracks.
  if (BG_TREE_TYPE_TRACK!=tree->vmt->type) {
    DVMESSAGE("unexpected tree type %d",tree->vmt->type);
    return "";
  }
  else {
#if defined (BG_WIN32_TARGET_UTF8) // [
    return track->target.oem.basename
        ?track->target.oem.basename:track->target.utf8.basename;
#else // ] [
    return track->target.oem.basename
        ?track->target.oem.basename:track->target.basename;
#endif // ]
  }
}

const wchar_t *bg_tree_out_basanamew(bg_tree_t *tree)
{
  return tree->target.basename;
}
#else // ] [
const char *bg_tree_in_basanamen(bg_tree_t *tree)
{
  return tree->source.basename;
}

const char *bg_tree_out_basanamen(bg_tree_t *tree)
{
  return tree->target.basename;
}
#endif // ]
