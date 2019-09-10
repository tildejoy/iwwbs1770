/*
 * bg_version.c
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
/*
 * Explore some constants at compile time and make them available at runtime.
 */
#if defined (__GNUC__) // [
#define UNUSED __attribute__((__unused__))
#else // ] [
#define UNUSED
#endif // ]

#if defined (_WIN32) // [
#include <windows.h>
#include <stdio.h>

int wmain(int argc UNUSED, wchar_t *const *argv UNUSED)
#else // ] [
#include <sys/utsname.h>
#if defined (__linux__) && defined (__GNUC__) // [
#include <gnu/libc-version.h>
#endif // ]
#include <string.h>
#include <stdio.h>

int main(int argc UNUSED, char *const *argv UNUSED)
#endif // ]
{
  FILE *f=stdout;
#if defined (_WIN32) // [
  OSVERSIONINFOA buf;

  memset(&buf,0,sizeof buf);
  buf.dwOSVersionInfoSize=sizeof buf;
  GetVersionExA(&buf);
#else // ] [
  struct utsname buf;

  memset(&buf,0,sizeof buf);
  uname(&buf);
#endif // ]

  fputs("#if ! defined (__BG_VERSION_H__) // [\n",f);
  fputs("#define __BG_VERSION_H__\n",f);
  fputs("/*\n",f);
  fputs(" * Some constants explored at compile time made available"
      " at runtime.\n",f);
  fputs(" */\n",f);
#if defined (_WIN32) // [
  fprintf(f,"#define BG_WINDOWS_MAJOR %lu\n",buf.dwMajorVersion);
  fprintf(f,"#define BG_WINDOWS_MINOR %lu\n",buf.dwMinorVersion);
  fprintf(f,"#define BG_WINDOWS_BUILD_NUMBER %lu\n",buf.dwBuildNumber);
  fprintf(f,"#define BG_WINDOWS_CSD_VESIONA \"%s\"\n",buf.szCSDVersion);
  fprintf(f,"#define BG_WINDOWS_CSD_VESIONW L\"%s\"\n",buf.szCSDVersion);
#else // ] [
  fprintf(f,"#define BG_POSIX_SYSNAME \"%s\"\n",buf.sysname);
  fprintf(f,"#define BG_POSIX_NODENAME \"%s\"\n",buf.nodename);
  fprintf(f,"#define BG_POSIX_RELEASE \"%s\"\n",buf.release);
  fprintf(f,"#define BG_POSIX_VERSION \"%s\"\n",buf.version);
  fprintf(f,"#define BG_POSIX_MACHINE \"%s\"\n",buf.machine);
#if defined (_GNU_SOURCE) // [
  fprintf(f,"#define BG_POSIX_DOMAINNAME \"%s\"\n",buf.domainname);
#endif // ]
#if defined (__linux__) && defined (__GNUC__) // [
  fputc('\n',f);
  fputs("#if defined (__linux__) && defined (__GNUC__) // [\n",f);
  fprintf(f,"#define BG_GNU_LIBC_VERSION \"%s\"\n",
      gnu_get_libc_version());
  fprintf(f,"#define BG_GNU_LIBC_RELEASE \"%s\"\n",
      gnu_get_libc_release());
  fputs("#endif // ]\n",f);
#endif // ]
#endif // ]
  fputc('\n',f);
  fputs("#endif // ]\n",f);

  return 0;
}
