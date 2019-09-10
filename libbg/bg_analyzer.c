/*
 * bg_analyzer.c
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
static bg_visitor_vmt_t bg_analyzer_vmt;

int bg_analyzer_create(bg_visitor_t *vis)
{
  /////////////////////////////////////////////////////////////////////////////
  vis->vmt=&bg_analyzer_vmt;
  vis->depth=0;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
}

static void bg_analyzer_destroy(bg_visitor_t *vis FFUNUSED)
{
}

///////////////////////////////////////////////////////////////////////////////
static int bg_analyzer_dispatch_file(bg_visitor_t *vis FFUNUSED,
    bg_tree_t *tree FFUNUSED)
{
	// file annotation is created bottom-up from the muxer. nothing to do.
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_analyzer_print_prefix(bg_visitor_t *vis, bg_tree_t *tree)
{
  FILE *f=tree->param->result.f;
  bg_print_vmt_t *stdout_vmt=tree->param->print.vmt;
  int stdout_prefix=BG_TREE_TYPE_TRACK==tree->vmt->type||stdout_vmt->infix;

  if (stdout_prefix) {
    ///////////////////////////////////////////////////////////////////////////
    // the header of a track is always mirrored to the shell/console
    // regardless wether prefix or infix.
    if (stdout_vmt->head(tree,vis->depth,stdout)<0) {
      DMESSAGE("printing head");
      goto e_print_head_stdout;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (tree->param->print.vmt->infix) {
    if (stdout!=f&&tree->param->print.vmt->head(tree,vis->depth,f)<0) {
      DMESSAGE("printing head");
      goto e_print_head_file_infix;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_print_head_file_infix:
e_print_head_stdout:
  return -1;
}

static int bg_analyzer_print_postfix(bg_visitor_t *vis, bg_tree_t *tree)
{
  FILE *f=tree->param->result.f;
#if 0 // [
  bg_print_vmt_t *stdout_vmt=&bg_print_classic_vmt;
#else // ] [
  bg_print_vmt_t *stdout_vmt=tree->param->print.vmt;
#endif // ]
  int stdout_prefix=BG_TREE_TYPE_TRACK==tree->vmt->type||stdout_vmt->infix;

  if (!stdout_prefix) {
    if (stdout_vmt->head(tree,vis->depth,stdout)<0) {
      DMESSAGE("printing head");
      goto e_print_head_stdout;
    }
  }

  if (!tree->param->print.vmt->infix) {
    if (stdout!=f&&tree->param->print.vmt->head(tree,vis->depth,f)<0) {
      DMESSAGE("printing head");
      goto e_print_head_file_postfix;
    }
  }

  if (tree->param->print.vmt->tail(tree,vis->depth,f)<0) {
    DMESSAGE("printing tail");
    goto e_print_tail_file;
  }

  fflush(f);

  if (stdout!=f) {
    if (tree->param->print.vmt->tail(tree,vis->depth,stdout)<0) {
      DMESSAGE("printing tail");
      goto e_print_tail_stdout;
    }

    fflush(stdout);
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_print_tail_stdout:
e_print_tail_file:
e_print_head_file_postfix:
e_print_head_stdout:
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_analyzer_dispatch_track(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;
  bg_track_t *track=&tree->track;

  /////////////////////////////////////////////////////////////////////////////
  ++vis->depth; // [

  /////////////////////////////////////////////////////////////////////////////
  if (ff_input_open_analyzer(&track->input)<0) {
    DMESSAGE("re-opening input");
    goto e_input;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (ff_analyzer_create(&track->analyzer,&track->input)<0) {
    DMESSAGE("creating analyzer");
    goto e_analyzer;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_print_prefix(vis,tree)<0) {
    DMESSAGE("printing prefix");
    goto e_print_prefix;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (ff_analyzer_loop(&tree->track.analyzer)<0) {
    DMESSAGE("decoding");
    goto e_decode;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_merge(tree->parent,tree)<0) {
    DMESSAGE("merging");
    goto e_merge;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_print_postfix(vis,tree)<0) {
    DMESSAGE("printing postfix");
    goto e_print_postfix;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_print_postfix:
e_merge:
e_decode:
e_print_prefix:
  ff_analyzer_destroy(&track->analyzer,0);
e_analyzer:
  ff_input_close(&track->input);
e_input:
  --vis->depth; // ]
  return err;
}

int bg_analyzer_album_prefix(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;
  bg_param_t *param=tree->param;

  /////////////////////////////////////////////////////////////////////////////
  ++vis->depth;

  if (param->output.dirname||param->overwrite) {
    if (tree->vmt->annotation.create(tree)<0) {
      DMESSAGE("annotating");
      goto e_annotate;
	  }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_print_prefix(vis,tree)<0) {
    DMESSAGE("printing prefix");
    goto e_print_prefix;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_print_prefix:
  if (err&&(param->output.dirname||param->overwrite))
    tree->vmt->annotation.destroy(tree);
e_annotate:
  if (err)
    --vis->depth;
  return err;
}

int bg_analyzer_album_suffix(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;
  bg_param_t *param=tree->param;
  bg_tree_t *cur;
  bg_visitor_t muxer;

  /////////////////////////////////////////////////////////////////////////////
  memset(&muxer,0,sizeof muxer);

  /////////////////////////////////////////////////////////////////////////////
  for (cur=tree->album.first;cur;cur=cur->next) {
    if (cur->vmt->accept(cur,vis)<0) {
      FFVMESSAGE("analyzing \"%s\"",cur->source.path);
      goto e_child;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_print_postfix(vis,tree)<0) {
    DMESSAGE("printing postfix");
    goto e_print_postfix;
  }

  ///////////////////////////////////////////////////////////////////////////
  if (tree->album.nleafs&&(param->output.dirname||param->overwrite)) {
    if (bg_muxer_create(&muxer)<0) {
      DMESSAGE("creating muxer");
      goto e_muxer;
    }

    if (!param->suppress.progress) {
      if (&bg_print_xml_vmt==param->print.vmt)
        fputs("<!-- ",stdout);

      fputs("begin remuxing ...\n",stdout);
      fflush(stdout);
    }

    if (tree->vmt->accept(tree,&muxer)<0) {
      DMESSAGE("muxing");
      goto e_muxer_accept;
    }

    if (!param->suppress.progress) {
      fputs("end remuxing.",stdout);

      if (&bg_print_xml_vmt==param->print.vmt)
        fputs(" -->",stdout);

      fputc('\n',stdout);
      fflush(stdout);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (param->output.dirname||param->overwrite)
    tree->vmt->annotation.destroy(tree);

  err=0;
//cleanup:
e_muxer_accept:
  if (muxer.vmt)
    muxer.vmt->destroy(&muxer);
e_muxer:
e_print_postfix:
e_child:
  --vis->depth;
  return err;
}

#if 0 // [
// never called. instead bg_analyzer_album_prefix() and
// bg_analyzer_album_suffix(), respectively, are called from bg_param_loop()
// and bg_pilot_callback_branch_leave(), respectively.
static int bg_analyzer_dispatch_album(bg_visitor_t *vis, bg_tree_t *tree)
{
  int err=-1;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_album_prefix(vis,tree)) {
    DMESSAGE("prefix");
    goto e_prefix;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_analyzer_album_suffix(vis,tree)) {
    DMESSAGE("suffix");
    goto e_suffix;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_merge(tree->parent,tree)<0) {
    DMESSAGE("merging");
    goto e_merge;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_merge:
e_suffix:
e_prefix:
  return err;
}
#else // ] [
static int bg_analyzer_dispatch_album(bg_visitor_t *vis FFUNUSED,
    bg_tree_t *tree FFUNUSED)
{
  DMESSAGE("non intended invocation ");
  return -1;
}
#endif // ]

static bg_visitor_vmt_t bg_analyzer_vmt={
#if defined (PBU_DEBUG) // [
  .id="analyzer",
#endif // ]
  .destroy=bg_analyzer_destroy,
  .dispatch_file=bg_analyzer_dispatch_file,
  .dispatch_track=bg_analyzer_dispatch_track,
  .dispatch_album=bg_analyzer_dispatch_album,
  .dispatch_root=bg_analyzer_dispatch_album,
};
