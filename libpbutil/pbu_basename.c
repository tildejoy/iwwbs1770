/*
 * pbu_basename.c
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
#include <pbutil.h>

#if defined (PBU_BASENAME_UNICODE) // [
static const char *pathnorm(const char *path)
{
  int len=strlen(path);
  char *sp=NULL;
  char *pp[2]={ 0 },*p;
  const char *ss;
  char clone[PATH_MAX];

#if defined (_WIN32) // [
  if (2<len&&':'==path[1]) {
    ss=path+2;
    len-=2;
  }
  else {
#endif // ]
    ss=path;
#if defined (_WIN32) // [
  }
#endif // ]

  if (PATH_MAX<len+1)
    return path;
  else {
    strcpy(clone,ss);

#if defined (_WIN32) // [
    for (p=strtok_r(clone,"\\",&sp);p;p=strtok_r(NULL,"\\",&sp)) {
      if (pp[0]) {
        while (!*pp[0])
          *pp[0]++='/';
      }

      pp[0]=p+strlen(p);
    }
#endif // ]

    // replace each '/' by a '\\' or a '/', respectively, and remove
    // trailing ones.
    for (p=strtok_r(clone,"/",&sp);p;p=strtok_r(NULL,"/",&sp)) {
      if (pp[0]) {
        while (!*pp[0])
          *pp[0]++='/';

        pp[1]=pp[0]+1;
      }

      pp[0]=p+strlen(p);
    }

    if (!pp[0])
      return path;
    else if (strlen(pp[0]))
      return path+(ss-path)+(pp[0]-clone);
    else if (pp[1])
      return path+(ss-path)+(pp[1]-clone);
    else
      return path;
  }
}
#endif // ]

const char *pbu_basename(const char *path)
{
#if ! defined (PBU_BASENAME_UNICODE) // [
  const char *p;
#endif // ]

  if (NULL==path)
    return NULL;

#if defined (PBU_BASENAME_UNICODE) // [
  return pathnorm(path);
#else // ] [
  p=path+strlen(path);

  // TODO: unicode.
  while (path<p&&('/'==p[-1]||'\\'==p[-1]))
    --p;

  // TODO: unicode.
  while (path<p&&('/'!=p[-1]&&'\\'!=p[-1]))
    --p;

  return p;
#endif // ]
}

#if defined (_WIN32) // [
const wchar_t *pbu_wbasename(const wchar_t *wpath)
{
#if 0 && defined (PBU_BASENAME_UNICODE) // [
  wchar_t *p;
#else // ] [
  const wchar_t *p;
#endif // ]

  if (NULL==wpath)
    return NULL;

  p=wpath+wcslen(wpath);

  // TODO: unicode.
  while (wpath<p&&('/'==p[-1]||'\\'==p[-1]))
    --p;

  // TODO: unicode.
  while (wpath<p&&('/'!=p[-1]&&'\\'!=p[-1]))
    --p;

  return p;
}
#endif // ]
