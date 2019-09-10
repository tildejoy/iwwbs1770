/*
 * bg_album.c
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
static bg_tree_vmt_t bg_album_vmt;

int bg_album_create(bg_tree_t **tree, bg_param_t *param, bg_tree_t *parent,
    const ffchar_t *path)
{

  /////////////////////////////////////////////////////////////////////////////
  *tree=malloc(sizeof **tree);

  if (!*tree) {
    DMESSAGE("allocating");
    goto e_alloc;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_common_create(*tree,param,parent,path)<0) {
    DMESSAGE("creating common");
    goto e_common;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_album_content_create(*tree,&bg_album_vmt)<0) {
    DMESSAGE("creating content");
    goto e_content;
  }

  /////////////////////////////////////////////////////////////////////////////
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
  bg_album_content_destroy(*tree);
e_content:
  bg_tree_common_destroy(*tree);
e_common:
  free(*tree);
e_alloc:
  return -1;
}

void bg_album_destroy(bg_tree_t *tree)
{
  if (tree->parent&&tree!=bg_album_pop(tree->parent))
    DWARNING("tree not at end of list");

#if defined (BG_PURGE) // [
  if (!tree->param->process&&1u<tree->album.nchildren) {
    // it's getting bottom-up.
    tree->param->argv.cur->purge=tree->depth+1;
  }
#endif // ]

  bg_album_content_destroy(tree);
  bg_tree_common_destroy(tree);
  free(tree);
}

int bg_album_content_create(bg_tree_t *tree, bg_tree_vmt_t *vmt)
{
  bg_album_t *album=&tree->album;

  tree->vmt=vmt;
#if defined (BG_PURGE) // [
  album->nchildren=0u;
#endif // ]
  album->nleafs=0u;
  album->first=NULL;
  album->last=NULL;

  return 0;
}

void bg_album_content_destroy(bg_tree_t *tree)
{
  bg_tree_t *cur;

  while (tree->album.last) {
    cur=bg_album_pop(tree);

    if (cur)
      cur->vmt->destroy(cur);
    else
      DWARNING("empty list");
  }
}

///////////////////////////////////////////////////////////////////////////////
int bg_album_push(bg_tree_t *tree, bg_tree_t *child)
{
  bg_album_t *album=&tree->album;

  if (album->last) {
    child->prev=album->last;
    album->last->next=child;
  }
  else
    album->first=child;

  album->last=child;
#if defined (BG_PURGE) // [
  ++album->nchildren;
#endif // ]

  return 0;
}

bg_tree_t *bg_album_pop(bg_tree_t *tree)
{
  bg_album_t *album=&tree->album;
  bg_tree_t *last=album->last;

  if (last) {
    last->parent=NULL;
    album->last=last->prev;

    if (album->last)
      album->last->next=NULL;
    else
      album->first=NULL;
  }
  else
    album->first=NULL;

  return last;
}

#if ! defined (BG_PURGE) // [
///////////////////////////////////////////////////////////////////////////////
const ffchar_t *bg_album_target_purge(bg_tree_t *tree)
{
  switch (tree->vmt->type) {
  case BG_TREE_TYPE_ALBUM:
#if 0 // [
  case BG_TREE_TYPE_ROOT:
#endif // ]
    break;
  default:
    DVMESSAGE("unexpected tree type: %d",tree->vmt->type);
    goto e_type;
  }

  return tree->source.basename;
e_type:
  return NULL;
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static int bg_album_accept(bg_tree_t *tree, bg_visitor_t *vis)
{
  return vis->vmt->dispatch_album(vis,tree);
}

static bg_tree_vmt_t bg_album_vmt={
#if defined (PBU_DEBUG) // [
  .id=FFL("album"),
#endif // ]
  .type=BG_TREE_TYPE_ALBUM,
  .destroy=bg_album_destroy,
  .accept=bg_album_accept,
  .annotation={
    .create=bg_album_annotation_create,
    .destroy=bg_album_annotation_destroy,
  },
};
