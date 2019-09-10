/*
 * bg_pathnorm.c
 *
 * Copyright (C) 2014-2019 Peter Belkner <pbelkner@users.sf.net>
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

ffchar_t *bg_pathnorm(ffchar_t *path)
{
#if defined (_WIN32) // [
  wchar_t *pp=path;

  while (*pp) {
    if (L'/'==*pp)
      *pp=L'\\';

    pp=CharNextW(pp);
  }
#endif // ]

  return path;
}
