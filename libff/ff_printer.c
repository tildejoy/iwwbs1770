/*
 * ff_printer.c
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

int ff_printer_create(ff_printer_t *p, FILE *f)
{
  p->f=f;
  p->len=0u;

  return 0;
}

void ff_printer_destroy(ff_printer_t *p FFUNUSED)
{
}


void ff_printer_clear(ff_printer_t *p)
{
  p->len=0u;
}

void ff_printer_reset(ff_printer_t *p)
{
  size_t i;

  for (i=0u;i<p->len;++i) {
#if (_WIN32) // [
    if (stdout!=p->f&&stderr!=p->f)
      fputwc(L'\b',p->f);
    else
#endif // ]
      fputc('\b',p->f);
  }

  p->len=0u;
}

void ff_printer_flush(ff_printer_t *p)
{
  size_t i;

  for (i=0u;i<p->len;++i) {
#if (_WIN32) // [
    if (stdout!=p->f&&stderr!=p->f)
      fputwc(L'\b',p->f);
    else
#endif // ]
      fputc('\b',p->f);
  }

  for (i=0u;i<p->len;++i) {
#if (_WIN32) // [
    if (stdout!=p->f&&stderr!=p->f)
      fputwc(L' ',p->f);
    else
#endif // ]
      fputc(' ',p->f);
  }

  for (i=0u;i<p->len;++i) {
#if (_WIN32) // [
    if (stdout!=p->f&&stderr!=p->f)
      fputwc(L'\b',p->f);
    else
#endif // ]
      fputc('\b',p->f);
  }

  p->len=0u;
  fflush(p->f);
}


enum { BUF_SIZE=64 };

int ff_printer_printf(ff_printer_t *p FFUNUSED, const char *format, ...)
{
  va_list ap;
  char buf[BUF_SIZE];

  va_start(ap,format);
#if 0 // [
  p->len=vsprintf(buf,format,ap);
#else // ] [
  p->len=vsnprintf(buf,BUF_SIZE,format,ap);
#endif // ]
  va_end(ap);
  fputs(buf,p->f);
  fflush(p->f);

  return p->len;
}

#if defined (_WIN32) // [
int ff_printer_wprintf(ff_printer_t *p FFUNUSED, const wchar_t *format, ...)
{
  va_list ap;
  wchar_t buf[BUF_SIZE];

  va_start(ap,format);
  p->len=vswprintf(buf,BUF_SIZE,format,ap);
  va_end(ap);
  fputws(buf,p->f);
  fflush(p->f);

  return p->len;
}
#endif // ]
