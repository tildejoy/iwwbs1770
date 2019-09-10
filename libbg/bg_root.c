/*
 * bg_root.h
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
static bg_tree_vmt_t bg_root_vmt;

int bg_root_create(bg_tree_t *tree, bg_param_t *param)
{
  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_common_create(tree,param,NULL,NULL)<0) {
    DMESSAGE("creating tree");
    goto e_common;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_album_content_create(tree,&bg_root_vmt)<0) {
    DMESSAGE("creating content");
    goto e_content;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  bg_album_content_destroy(tree);
e_content:
  bg_tree_common_destroy(tree);
e_common:
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
static void bg_root_destroy(bg_tree_t *tree)
{
  bg_album_content_destroy(tree);
  bg_tree_common_destroy(tree);
}

static int bg_root_accept(bg_tree_t *tree, bg_visitor_t *vis)
{
  return vis->vmt->dispatch_root(vis,tree);
}

static bg_tree_vmt_t bg_root_vmt={
#if defined (PBU_DEBUG) // [
  .id=FFL("root"),
#endif // ]
  .type=BG_TREE_TYPE_ROOT,
  .destroy=bg_root_destroy,
  .accept=bg_root_accept,
  .annotation={
    .create=bg_root_annotation_create,
    .destroy=bg_root_annotation_destroy,
  },
};
