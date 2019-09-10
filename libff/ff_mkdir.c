/*
 * ff_mkdir.c
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
#if defined (_WIN32) // [
#include <direct.h>
//#include <sys/stat.h>

//#define FFSEPSTRING FFL("\\")
//#define FFSTRTOK(str,delim,saveptr) wcstok(str,delim)
//#define FFSTAT(path,buf) _wstat(path,buf)
//#define FFMKDIR(path) _wmkdir(path)

//typedef struct _stat ffstat_t;
#else // ] [
//#include <unistd.h>
//#include <sys/types.h>
#include <sys/stat.h>

//#define FFSEPSTRING FFL("/")
//#define FFSTRTOK(str,delim,saveptr) strtok_r(str,delim,saveptr)
//#define FFSTAT(path,buf) stat(path,buf)
//#define FFMKDIR(path) mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)

typedef struct stat ffstat_t;
#endif // ]

/*
int fffexists(const ffchar_t *path)
{
  ffstat_t statbuf;

  return FFSTAT(path,&statbuf)<0?0:1;
}
*/

/*
int fffcmp(const ffchar_t *lpath, const ffchar_t *rpath)
{
  ffstat_t lhs,rhs;
  int cmp;

  if (FFSTAT(lpath,&lhs)<0)
    return -1;
  else if (FFSTAT(rpath,&rhs)<0)
    return 1;
  else if ((cmp=lhs.st_dev-rhs.st_dev))
    return cmp;
  else if ((cmp=lhs.st_ino-rhs.st_ino))
    return cmp;
  else
    return 0;
}
*/

int ff_mkdir(ffchar_t *path)
{
  ffchar_t *pp=path;

  if (!path)
    goto success;
#if defined (_WIN32) // [
  else if (1<wcslen(pp)&&L':'==pp[1])
    pp+=2;
#endif // ]

  for (;;) {
#if defined (_WIN32) // [
    pp=wcsstr(pp,L"\\");
#else // ] [
    pp=strstr(pp,"/");
#endif // ]

    if (pp) {
      *pp=FFL('\0');
#if defined (_WIN32) // [
      _wmkdir(path);
#else // ] [
      mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
#endif // ]
      *pp=FFPATHSEP;
      ++pp;
    }
    else
      break;
  }

#if defined (_WIN32) // [
  _wmkdir(path);
#else // ] [
  mkdir(path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
#endif // ]
success:
  return 0;
}
