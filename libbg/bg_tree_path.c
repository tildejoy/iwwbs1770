/*
 * bg_tree_path.c
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

int bg_tree_source_create(bg_tree_path_t *tp, const ffchar_t *path)
{
  if (path) {
    tp->path=FFSTRDUP(path);

    if (!tp->path) {
      DMESSAGE("duplicating path");
      goto e_path;
    }

    ///////////////////////////////////////////////////////////////////////////
    tp->basename=bg_basename(tp->path);
  }
  else {
    tp->path=NULL;
    tp->basename=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (tp->path)
    free(tp->path);
e_path:
  return -1;
}

void bg_tree_path_destroy(bg_tree_path_t *tp)
{
  if (tp->path)
    free(tp->path);
}
