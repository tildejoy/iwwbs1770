/*
 * pbu_vwritelnw.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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
#include <stdarg.h>
#include <pbutil.h>

#if defined (_WIN32) // [
void pbu_vwritelnw(FILE *f, const char *path, int line, const char *func,
    const wchar_t *format, ...)
{
  va_list ap;
#if ! defined (PBU_BASENAME_UNICODE) // [
  const char *p;
#endif // ]

  if (format) {
    va_start(ap,format);
    vfwprintf(f,format,ap);
    va_end(ap);
  }

  if (path) {
    if (format)
      fputs(" (",f);

    path=pbu_basename(path);
#if defined (PBU_BASENAME_UNICODE) // [
    fprintf(f,"%s:%d:%s",path,line,func);
#else // ] [
    p=path+strlen(path);

    while (path<p&&'/'!=p[-1]&&'\\'!=p[-1])
      --p;

    fprintf(f,"%s:%d:%s",p,line,func);
#endif // ]

    if (format)
      fputc(')',f);
  }

  fputc('\n',f);
  fflush(f);
}
#endif // ]
