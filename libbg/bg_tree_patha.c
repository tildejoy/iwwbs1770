/*
 * bg_tree_patha.c
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

#if defined (_WIN32) // [
int bg_tree_patha_create(bg_tree_patha_t *p, const wchar_t *path,
    unsigned int codepage)
{
  if (path) {
    ///////////////////////////////////////////////////////////////////////////
    p->path=bg_wcs2str(path,codepage);

    if (!p->path) {
      DMESSAGE("creating utf-8 representation of path");
      goto epath;
    }

    ///////////////////////////////////////////////////////////////////////////
    p->basename=bg_basenamea(p->path);
  }
  else {
    p->path=NULL;
    p->basename=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (p->path)
    free(p->path);
epath:
  return -1;
}

void bg_tree_patha_destroy(bg_tree_patha_t *p)
{
  if (p->path)
    free(p->path);
}
#endif // ]
