/*
 * ff_mv.c
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
#include <ff.h>

int ff_mv(const ffchar_t *source, const ffchar_t *target)
{
#if defined (_WIN32) // [
  DWORD dwError;

  DeleteFileW(target);

  if (!MoveFileW(source,target)) {
    dwError=GetLastError();

    switch (dwError) {
    case ERROR_ALREADY_EXISTS:
      DVMESSAGEW(L"moving \"%s\" to \"%s\" (ERROR_ALREADY_EXISTS)",
          source,target);
      break;
    default:
      DVMESSAGEW(L"moving \"%s\" to \"%s\" (%lu)",
          source,target,dwError);
      break;
    }

    goto e_rename;
  }
#else // ] [
  remove(target);

  if (rename(source,target)<0) {
    DVMESSAGE("moving \"%s\" to \"%s\"",source,target);
    goto e_rename;
  }
#endif // ]
  return 0;
//cleanup:
e_rename:
  return -1;
}
