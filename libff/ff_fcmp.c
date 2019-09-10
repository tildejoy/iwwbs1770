/*
 * ff_fcmp.c
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
#if ! defined (_WIN32) // [
#include <sys/stat.h>
#endif // ]

#if defined (_WIN32) // [
int ff_get_info(const wchar_t *path, BY_HANDLE_FILE_INFORMATION *info)
{
  int err=-1;
  HANDLE h;
  int ok;

  /////////////////////////////////////////////////////////////////////////////
  h=CreateFileW(
    path,               // LPCWSTR               lpFileName,
    GENERIC_READ,       // DWORD                 dwDesiredAccess,
    FILE_SHARE_READ,    // DWORD                 dwShareMode,
    NULL,               // LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OPEN_EXISTING,
                        // DWORD                 dwCreationDisposition,
    FILE_ATTRIBUTE_NORMAL,
                        // DWORD                 dwFlagsAndAttributes,
    NULL                // HANDLE                hTemplateFile
  );

  if (INVALID_HANDLE_VALUE==h)
    goto e_open;

  /////////////////////////////////////////////////////////////////////////////
  ok=GetFileInformationByHandle(
    h,              // HANDLE                       hFile,
    info            // LPBY_HANDLE_FILE_INFORMATION lpFileInformation
  );

  if (!ok)
    goto e_info;

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_info:
  CloseHandle(h);
e_open:
  return err;
}
#endif // ]

int ff_fcmp(const ffchar_t *lhs, const ffchar_t *rhs)
{
  int cmp;
#if defined (_WIN32) // [
	// How to determine that two Win32 API handles represent the same object?
	// https://stackoverflow.com/questions/29436696/how-to-determine-that-two-win32-api-handles-represent-the-same-object
  struct {
	  BY_HANDLE_FILE_INFORMATION lhs,rhs;
  } info={ 0 };

//FFVWRITELN(FFL("\"%s\" \"%s\""),lhs,rhs);
  if (ff_get_info(lhs,&info.lhs)<0)
    return -1;
  else if (ff_get_info(rhs,&info.rhs)<0)
    return 1;
  else if ((cmp=info.lhs.dwVolumeSerialNumber-info.rhs.dwVolumeSerialNumber))
    return cmp;
  else if ((cmp=info.lhs.nFileIndexHigh-info.rhs.nFileIndexHigh))
    return cmp;
  else if ((cmp=info.lhs.nFileIndexLow-info.rhs.nFileIndexLow))
    return cmp;
  else
    return 0;
#else // ] [
  struct {
    struct stat lhs,rhs;
  } buf={ 0 };

  if (stat(lhs,&buf.lhs)<0)
    return -1;
  else if (stat(rhs,&buf.rhs)<0)
    return 1;
  else if ((cmp=buf.lhs.st_dev-buf.rhs.st_dev))
    return cmp;
  else if ((cmp=buf.lhs.st_ino-buf.rhs.st_ino))
    return cmp;
  else
    return 0;
#endif // ]
}
