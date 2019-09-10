/*
 * bg_file.c
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
static bg_tree_vmt_t bg_file_vmt;

///////////////////////////////////////////////////////////////////////////////
int bg_file_content_create(bg_tree_t *tree)
{
  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
  // DON'T do this!!! bg_file_create() is called from bg_child_create()
  // which in front already has called bg_tree_common_create() and just
  // leaves us with setting the vmt!
  if (bg_tree_common_create(tree,param,param,path,&bg_file_vmt)<0) {
    DMESSAGE("creating tree");
    goto etree;
  }
#else // ] [
  if ((BG_FLAGS_EXT_CSV&tree->param->flags.extension)
      &&!FFSTRCMP(FFL("folder.csv"),tree->source.basename))
    goto e_basename;

  tree->vmt=&bg_file_vmt;
#endif // ]

  ++tree->parent->album.nleafs;

  return 0;
//cleanup:
e_basename:
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
static void bg_file_destroy(bg_tree_t *tree)
{
#if defined (BG_PURGE) // [
  bg_param_t *param=tree->param;
#endif // ]

  if (tree->parent)
    bg_album_pop(tree->parent);

#if defined (BG_PURGE) // [
  if (!param->process)
    param->argv.cur->purge=tree->depth;
#endif // ]

  bg_tree_common_destroy(tree);
  free(tree);
}

static int bg_file_accept(bg_tree_t *tree, bg_visitor_t *vis)
{
  return vis->vmt->dispatch_file(vis,tree);
}

static bg_tree_vmt_t bg_file_vmt={
#if defined (PBU_DEBUG) // [
  .id=FFL("file"),
#endif // ]
  .type=BG_TREE_TYPE_FILE,
  .destroy=bg_file_destroy,
  .accept=bg_file_accept,
  .annotation={
    .create=bg_file_annotation_create,
    .destroy=bg_file_annotation_destroy,
  },
};
