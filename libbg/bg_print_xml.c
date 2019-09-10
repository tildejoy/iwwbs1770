/*
 * bs1770gain_print_xml.c
 *
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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
#if ! defined (_WIN32) // [
#include <ctype.h>

static char *strlwr(char *s)
{
  char *sp;

  for (sp=s;*sp;++sp)
    *sp=tolower(*sp);

  return s;
}
#endif // ]

static void bg_print_xml_encoding(bg_param_t *param, int bits)
{
  FILE *f=param->result.f;

#if defined (WIN32) // [
  if (stdin==f||stdout==f) {
#endif // ]
    fprintf(f,"<?xml version=\"1.0\" encoding=\"UTF-%d\""
        " standalone=\"no\"?>\n",bits);
#if defined (WIN32) // [
  }
  else {
    fwprintf(f,L"<?xml version=\"1.0\" encoding=\"UTF-%d\""
        " standalone=\"no\"?>\n",bits);
  }
#endif // ]
}

static void bg_print_xml_indent(int depth, FILE *f)
{
  enum { SIZE=128 };
  
  union {
    // narrow string representation.
    char n[SIZE];
#if defined (WIN32) // [
    // wide string representation.
    wchar_t w[SIZE];
#endif // ]
  } format;

  if (--depth) {
    depth*=2;

    // indentation.
#if defined (WIN32) // [
    if (stdin==f||stdout==f) {
#endif // ]
      snprintf(format.n,SIZE,"%%%ds",depth);
      fprintf(f,format.n,"");
#if defined (WIN32) // [
    }
    else {
      _snwprintf(format.w,SIZE,L"%%%ds",depth);
      fwprintf(f,format.w,L"");
    }
#endif // ]
  }
}

static int bg_print_xml_head(bg_tree_t *tree, int depth, FILE *f)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;
  
  switch (tree->vmt->type) {
  case BG_TREE_TYPE_TRACK:
    bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
    if (stdin!=f&&stdout!=f) {
      fwprintf(f,L"<track total=\"%lu\" number=\"%lu\" file=\"%s\">\n",
          param->count.max,track->root.id,bg_tree_in_basanamew(tree));
    }
    else {
#endif // ]
      fprintf(f,"<track total=\"%lu\" number=\"%lu\" file=\"%s\">\n",
          param->count.max,track->root.id,bg_tree_in_basanamen(tree));
#if defined (WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!param->suppress.hierarchy) {
      bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
      if (stdin!=f&&stdout!=f)
        fwprintf(f,L"<album folder=\"%s\">\n",bg_tree_in_basanamew(tree));
      else
#endif // ]
        fprintf(f,"<album folder=\"%s\">\n",bg_tree_in_basanamen(tree));
    }

    break;
  case BG_TREE_TYPE_ROOT:
    bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
    if (stdin!=f&&stdout!=f)
      fwprintf(f,L"<bs1770gain norm=\"%0.2f\">\n",param->norm);
    else
#endif // ]
      fprintf(f,"<bs1770gain norm=\"%0.2f\">\n",param->norm);

    break;
  default:
    DMESSAGE("unknown type");
    goto e_type;
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);

  return 0;
//cleanup:
e_type:
  return -1;
}

static void bg_print_conf_tail(bg_print_conf_t *c, bg_tree_t *tree,
    int depth, FILE *f)
{
  enum { SIZE_UNIT=64 };

  union {
#if defined (_WIN32) // [
    wchar_t w[SIZE_UNIT];
#endif // ]
    char m[SIZE_UNIT];
  } unit;

  switch (c->argc) {
  case 1:
    if (c->argv[0]) {
      bg_print_xml_indent(depth,f);

#if defined (_WIN32) // [
      // when not writing to the console/shell (i.e. to a file) we need to
      // use wide otherwise narrow character strings.
      if (stdin!=f&&stdout!=f) {
        wcsncpy(unit.w,c->w.unit(tree),SIZE_UNIT-1);
        _wcslwr(unit.w);
        fwprintf(f,c->w.format.xml,unit.w,c->argv[0](tree));
      }
      else {
#endif // ]
        strncpy(unit.m,c->n.unit(tree),SIZE_UNIT-1);
        strlwr(unit.m);
        fprintf(f,c->n.format.xml,unit.m,c->argv[0](tree));
#if defined (_WIN32) // [
      }
#endif // ]
    }
    else
      DWARNING("argv[0]");

    break;
  case 2:
    if (c->argv[0]&&c->argv[1]) {
      bg_print_xml_indent(depth,f);

#if defined (_WIN32) // [
      // when not writing to the console/shell (i.e. to a file) we need to
      // use wide otherwise narrow character strings.
      if (stdin!=f&&stdout!=f) {
        wcsncpy(unit.w,c->w.unit(tree),SIZE_UNIT-1);
        _wcslwr(unit.w);

        if (c->unitc<2) {
          fwprintf(f,c->w.format.xml,unit.w,c->argv[0](tree),
              unit.w,c->argv[1](tree));
        }
        else {
          fwprintf(f,c->w.format.xml,unit.w,c->argv[0](tree),
              c->argv[1](tree));
        }
      }
      else {
#endif // ]
        strncpy(unit.m,c->n.unit(tree),SIZE_UNIT-1);
        strlwr(unit.m);

        if (c->unitc<2) {
          fprintf(f,c->n.format.xml,unit.m,c->argv[0](tree),
              c->argv[1](tree));
        }
        else {
          fprintf(f,c->n.format.xml,unit.m,c->argv[0](tree),
              unit.m,c->argv[1](tree));
        }
#if defined (_WIN32) // [
      }
#endif // ]
    }
    else
      DWARNING("argv[0]/argv[1]");

    break;
  default:
    DWARNING("argc");
    break;
  }
}

static int bg_print_xml_tail(bg_tree_t *tree, int depth, FILE *f)
{
  bg_param_t *param=tree->param;
  bg_flags_agg_t agg;
  bg_print_conf_t *c;

  if (BG_TREE_TYPE_TRACK==tree->vmt->type||!param->suppress.hierarchy) {
    for (agg=1,c=bg_print_conf;agg<BG_FLAGS_AGG_MAX;agg<<=1,++c) {
      if (!(agg&tree->param->flags.aggregate)) {
        // this aggregation isn't involved.
        continue;
      }
      else if (agg!=c->aggregate) {
        // wrong order.
        DWARNING("aggregate mismatch");
        continue;
      }
      else
        bg_print_conf_tail(c,tree,depth+1,f);
    }
  }

  switch (tree->vmt->type) {
  case BG_TREE_TYPE_TRACK:
    bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
    if (stdin==f||stdout==f)
#endif // ]
      fprintf(f,"</track>\n");
#if defined (WIN32) // [
    else
      fwprintf(f,L"</track>\n");
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!param->suppress.hierarchy) {
      bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
      if (stdin==f||stdout==f)
#endif // ]
        fprintf(f,"</album>\n");
#if defined (WIN32) // [
      else
        fwprintf(f,L"</album>\n");
#endif // ]
    }

    break;
  case BG_TREE_TYPE_ROOT:
    bg_print_xml_indent(depth,f);

#if defined (WIN32) // [
    if (stdin==f||stdout==f)
#endif // ]
      fprintf(f,"</bs1770gain>\n");
#if defined (WIN32) // [
    else
      fwprintf(f,L"</bs1770gain>\n");
#endif // ]

    break;
  default:
    DMESSAGE("unknown type");
    goto e_type;
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);

  return 0;
//cleanup:
e_type:
  return -1;
}

bg_print_vmt_t bg_print_xml_vmt={
  .id="xml",
  .infix=1,
  .encoding=bg_print_xml_encoding,
  .head=bg_print_xml_head,
  .tail=bg_print_xml_tail,
};
