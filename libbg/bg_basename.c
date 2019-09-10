/*
 * bg_basename
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

const ffchar_t *bg_basename(const ffchar_t *path)
{
  const ffchar_t *bn=path;
  const ffchar_t *basename;

#if defined (_WIN32) // [
  if (1<wcslen(bn)&&L':'==bn[1]) {
    bn+=2;

    if (L'\\'==*bn)
      ++bn;
  }
#endif // ]

  basename=bn;

  for (;;) {
#if defined (_WIN32) // [
    bn=wcsstr(bn,L"\\");
#else // ] [
    bn=strstr(bn,"/");
#endif // ]

    if (bn)
      basename=++bn;
    else
      break;
  }

  return basename;
}
