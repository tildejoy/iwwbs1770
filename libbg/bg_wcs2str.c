/*
 * bg_wcs2str.c
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
char *bg_wcs2str(const wchar_t *wcs, unsigned int codepage)
{
  size_t size;
  char *str;

  ///////////////////////////////////////////////////////////////////////////
  size=WideCharToMultiByte(
    codepage,       // _In_      UINT    CodePage,
    0,              // _In_      DWORD   dwFlags,
    wcs,            // _In_      LPCWSTR lpWideCharStr,
    -1,             // _In_      int     cchWideChar,
    NULL,           // _Out_opt_ LPSTR   lmultiByteStr,
    0,              // _In_      int     cbMultiByte,
    NULL,           // _In_opt_  LPCSTR  lpDefaultChar,
    NULL            // _Out_opt_ LPBOOL  lpUsedDefaultChar
  );

  str=malloc(size*sizeof *str);

  if (!str) {
    DMESSAGE("allocating");
    goto ealloc;
  }

  ///////////////////////////////////////////////////////////////////////////
  WideCharToMultiByte(
    codepage,       // _In_      UINT    CodePage,
    0,              // _In_      DWORD   dwFlags,
    wcs,            // _In_      LPCWSTR lpWideCharStr,
    -1,             // _In_      int     cchWideChar,
    str,            // _Out_opt_ LPSTR   lmultiByteStr,
    size,           // _In_      int     cbMultiByte,
    NULL,           // _In_opt_  LPCSTR  lpDefaultChar,
    NULL            // _Out_opt_ LPBOOL  lpUsedDefaultChar
  );

  return str;
ealloc:
  return NULL;
}
#endif // ]
