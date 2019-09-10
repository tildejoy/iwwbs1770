/*
 * bg_pilot_branch.c
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
static bg_pilot_hist_vmt_t bg_pilot_hist_branch_vmt;

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
int bg_pilot_hist_branch_create(bg_pilot_hist_t *hist, HANDLE hFind,
    WIN32_FIND_DATAW *e)
#else // ] [
int bg_pilot_hist_branch_create(bg_pilot_hist_t *hist, DIR *dir)
#endif // ]
{
  bg_pilot_client_t *client=&hist->pilot->client;
  bg_pilot_hist_branch_t *branch=&hist->branch;

  hist->vmt=&bg_pilot_hist_branch_vmt;
#if defined (_WIN32) // [
  branch->hFind=hFind;
  branch->e=*e;
#else // ] [
  branch->dir=dir;
  branch->e=NULL;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (client->cb&&client->cb->branch.enter) {
    if (client->cb->branch.enter(hist,client->data)<0) {
      DMESSAGE("entering branch");
      goto e_enter;
    }
  }

  return 0;
//cleanup:
e_enter:
#if defined (_WIN32) // [
  FindClose(hFind);
#else // ] [
  closedir(dir);
#endif // ]
  return -1;
}

static void bg_pilot_hist_branch_destroy(bg_pilot_hist_t *hist)
{
  bg_pilot_client_t *client=&hist->pilot->client;
  bg_pilot_hist_branch_t *branch=&hist->branch;

  if (client->cb&&client->cb->branch.leave)
    client->cb->branch.leave(hist,client->data);

#if defined (_WIN32) // [
  FindClose(branch->hFind);
#else // ] [
  closedir(branch->dir);
#endif // ]
}

///////////////////////////////////////////////////////////////////////////////
static int bg_pilot_hist_branch_navdir(bg_pilot_hist_t *hist)
{
  bg_pilot_hist_branch_t *branch=&hist->branch;

#if defined (_WIN32) // [
  if (!wcscmp(L".",branch->e.cFileName))
    return 1;
  else if (!wcscmp(L"..",branch->e.cFileName))
    return 1;
#else // ] [
  if (!strcmp(".",branch->e->d_name))
    return 1;
  else if (!strcmp("..",branch->e->d_name))
    return 1;
#endif // ]
  else
    return 0;
}

static const ffchar_t *bg_pilot_hist_branch_first(bg_pilot_hist_t *hist)
{
  const ffchar_t *entry=NULL;
  bg_pilot_hist_branch_t *branch=&hist->branch;

#if defined (_WIN32) // [
  while (bg_pilot_hist_branch_navdir(hist)) {
    if (!FindNextFileW(branch->hFind,&branch->e))
      goto e_readdir;
  }

  entry=branch->e.cFileName;
#else // ] [
  do {
    branch->e=readdir(branch->dir);

    if (!branch->e)
      goto e_readdir;
  } while (bg_pilot_hist_branch_navdir(hist));

  entry=branch->e->d_name;
#endif // ]
//cleanup:
e_readdir:
  return entry;
}

static const ffchar_t *bg_pilot_hist_branch_next(bg_pilot_hist_t *hist)
{
  const ffchar_t *entry=NULL;
  bg_pilot_hist_branch_t *branch=&hist->branch;

#if defined (_WIN32) // [
  do {
    if (!FindNextFileW(branch->hFind,&branch->e))
      goto e_readdir;
  } while (bg_pilot_hist_branch_navdir(hist));

  entry=branch->e.cFileName;
#else // ] [
  do {
    branch->e=readdir(branch->dir);

    if (!branch->e)
      goto e_readdir;
  } while (bg_pilot_hist_branch_navdir(hist));

  entry=branch->e->d_name;
#endif // ]
//cleanup:
e_readdir:
  return entry;
}

///////////////////////////////////////////////////////////////////////////////
static bg_pilot_hist_vmt_t bg_pilot_hist_branch_vmt={
#if defined (PBU_DEBUG) // [
  .id="branch",
#endif // ]
  .destroy=bg_pilot_hist_branch_destroy,
  .first=bg_pilot_hist_branch_first,
  .next=bg_pilot_hist_branch_next,
};
