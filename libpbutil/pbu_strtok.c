/*
 * pbu_strtok.c
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
#if defined (_WIN32) // {
#include <pbutil.h>

static char *pbu_strtok(char *str, const char *delim, char **saveptr)
{
  (void)saveptr;

  return strtok(str,delim);
}

char *pbu_strtok_r(char *str, const char *delim, char **saveptr)
{
  typedef char *(*strtok_s_t)(char *,const char *,char **); 
  static strtok_s_t strtok_s=NULL;
  HANDLE hLib;

  if (NULL==strtok_s) {
    if (NULL==(hLib=pbu_msvcrt()))
      goto strtok;

#if 0 && (! defined (__GNUC__) || ! defined (_WIN64)) // [
    if (NULL==(strtok_s=(strtok_s_t)GetProcAddress(hLib,"strtok_s")))
      goto strtok;
#else // ] [
    if (NULL==(strtok_s=(void *)GetProcAddress(hLib,"strtok_s")))
      goto strtok;
#endif // ]

    goto strtok_s;
  strtok:
    strtok_s=pbu_strtok;
    goto strtok_s;
  }
strtok_s:
  return strtok_s(str,delim,saveptr);
}
#endif // }
