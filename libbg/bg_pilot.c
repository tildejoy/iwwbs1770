/*
 * bg_pilot.c
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
#include <bg_pilot.h>

///////////////////////////////////////////////////////////////////////////////
int bg_pilot_create(bg_pilot_t *pilot, size_t size, bg_pilot_callback_t *cb,
    void *data)
{
  /////////////////////////////////////////////////////////////////////////////
  pilot->client.cb=cb;
  pilot->client.data=data;

  /////////////////////////////////////////////////////////////////////////////
  pilot->min=malloc(size*sizeof pilot->min[0]);
  
  if (!pilot->min) {
    DMESSAGE("allocating");
    goto e_malloc;
  }

  /////////////////////////////////////////////////////////////////////////////
  pilot->nxt=pilot->min;
  pilot->max=pilot->min+size;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  free(pilot->min);
e_malloc:
  return -1;
}

static int bg_pilot_realloc(bg_pilot_t *pilot)
{
  size_t size=pilot->max-pilot->min;
  size_t offs=pilot->nxt-pilot->min;
  bg_pilot_hist_t *min;

  /////////////////////////////////////////////////////////////////////////////
  size<<=1;

  if (!size) {
    DMESSAGE("overflow");
    goto e_overflow;
  }

  /////////////////////////////////////////////////////////////////////////////
  min=realloc(pilot->min,size*sizeof min[0]);

  if (!min) {
    DMESSAGE("reallocating");
    goto e_realloc;
  }

  pilot->min=min;
  pilot->nxt=min+offs;
  pilot->max=min+size;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_realloc:
e_overflow:
  return -1;
}

void bg_pilot_destroy(bg_pilot_t *pilot)
{
  while (pilot->min<pilot->nxt) {
    --pilot->nxt;
    pilot->nxt->vmt->destroy(pilot->nxt);
  }

  free(pilot->min);
}

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
static wchar_t *pathcpyw(wchar_t *target, const wchar_t *source)
{
  wchar_t *tp=target;

  while (*source) {
    switch (*source) {
    case L'/':
      *tp++=L'\\';
      ++source;
      break;
    default:
      *tp++=*source++;
      break;
    }
  }

  *tp=0;

  return target;
}
#endif // ]

static int bg_pilot_push(bg_pilot_t *pilot, const ffchar_t *path)
{
  bg_pilot_hist_t *cur=pilot->min<pilot->nxt?pilot->nxt-1:NULL;
  const ffchar_t *root=cur?cur->path:NULL;
  size_t len1=root?FFSTRLEN(root):0u;
  size_t len2=path?FFSTRLEN(path):0u;
  size_t size=(len1?len1+1:0)+(len2?len2+1:0);
  ffchar_t *pp;
#if defined (_WIN32) // [
  wchar_t *mask,*mp;
  HANDLE hFind;
  WIN32_FIND_DATAW e;
#else // ] [
  DIR *dir;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (!size) {
    DMESSAGE("size");
    goto e_size;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (pilot->max==pilot->nxt&&bg_pilot_realloc(pilot)<0) {
    DMESSAGE("reallocating");
    goto e_realloc;
  }

  cur=pilot->nxt;
  ++pilot->nxt;

  /////////////////////////////////////////////////////////////////////////////
  cur->path=pp=malloc(size*sizeof cur->path[0]);

  if (!cur->path) {
    DMESSAGE("allocatig path");
    goto e_path;
  }

  cur->pilot=pilot;

#if defined (_WIN32) // [
  if (len1) {
    pathcpyw(pp,root);
    pp+=len1;

    if (len2)
      *pp++=FFPATHSEP;
  }

  if (len2) {
    pathcpyw(pp,path);
    pp+=len2;
  }

  while (cur->path<pp&&FFPATHSEP==pp[-1])
    *--pp=L'\0';

  size+=2;
  mask=mp=malloc(size*sizeof mask[0]);

  if (!mask) {
    DMESSAGE("allocating directory mask");
    goto e_mask;
  }

  wcscpy(mp,cur->path);
  mp+=wcslen(mask);
  *mp++=FFPATHSEP;
  *mp++=L'*';
  *mp++=L'\0';
  memset(&e,0,sizeof e);
  hFind=FindFirstFileW(mask,&e);
  free(mask);

  if (INVALID_HANDLE_VALUE==hFind) {
    if (bg_pilot_hist_leaf_create(cur)<0) {
      DMESSAGE("creating leaf");
      goto e_entry;
    }
  }
  else {
    if (bg_pilot_hist_branch_create(cur,hFind,&e)<0) {
      DMESSAGE("creating branch");
      goto e_entry;
    }
  }
#else // ] [
  if (len1) {
    strcpy(pp,root);
    pp+=len1;

    if (len2)
      *pp++=FFPATHSEP;
  }

  if (len2) {
    strcpy(pp,path);
    pp+=len2;
  }

  while (cur->path<pp&&FFPATHSEP==pp[-1])
    *--pp='\0';

  dir=opendir(cur->path);

  if (dir) {
    if (bg_pilot_hist_branch_create(cur,dir)<0) {
      DMESSAGE("creating branch");
      goto e_entry;
    }
  }
  else {
    if (bg_pilot_hist_leaf_create(cur)<0) {
      DMESSAGE("creating leaf");
      goto e_entry;
    }
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_entry:
#if defined (_WIN32) // [
e_mask:
#endif // ]
  if (cur)
    free(cur->path);
e_path:
  --pilot->nxt;
e_realloc:
e_size:
  return -1;
}

static void bg_pilot_pop(bg_pilot_t *pilot)
{
  bg_pilot_hist_t *cur=pilot->min<pilot->nxt?pilot->nxt-1:NULL;

  if (cur) {
    cur->vmt->destroy(cur);

    if (cur->path)
      free(cur->path);

    --pilot->nxt;
  }
}

int bg_pilot_first(bg_pilot_t *pilot, const ffchar_t *path)
{
  int err=-1;
  bg_pilot_hist_t *cur;

  /////////////////////////////////////////////////////////////////////////////
  do {
    if (bg_pilot_push(pilot,path)<0) {
      DMESSAGE("pushing");
      goto e_push;
    }

    cur=pilot->nxt-1;
    path=cur->vmt->first(cur);
  } while (path);

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_push:
  return err;
}

int bg_pilot_next(bg_pilot_t *pilot, int size)
{
  int err=-1;
  bg_pilot_hist_t *cur;
  const ffchar_t *entry;

  /////////////////////////////////////////////////////////////////////////////
  while (size<pilot->nxt-pilot->min) {
    cur=pilot->nxt-1;
    entry=cur->vmt->next(cur);

    if (entry) {
      do {
        if (bg_pilot_push(pilot,entry)<0) {
          DMESSAGE("pushing");
          goto e_push;
        }

        cur=pilot->nxt-1;
        entry=cur->vmt->first(cur);
      } while (entry);

      break;
    }
    else
      bg_pilot_pop(pilot);
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_push:
  return err;
}

int bg_pilot_loop(bg_pilot_t *pilot, const ffchar_t *path)
{
  int err=-1;
  int size=pilot->nxt-pilot->min;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_pilot_first(pilot,path)<0) {
    DMESSAGE("first");
    goto e_first;
  }

  /////////////////////////////////////////////////////////////////////////////
  while (size<pilot->nxt-pilot->min) {
    if (bg_pilot_next(pilot,size)<0) {
      DMESSAGE("next");
      goto e_next;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_next:
e_first:
  return err;
}

int bg_pilot_empty(const bg_pilot_t *pilot)
{
  return pilot->min==pilot->nxt;
}
