/*
 * bg_pilot.h
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
#if ! defined (__BG_PILOT_H__) // [
#define __BG_PILOT_H__
#if defined (_WIN32) // [
#include <windows.h>
#else // ] [
#include <dirent.h>
#endif // ]
#include <ff.h>

#if defined (__cplusplus) // [
extern "C" {
#endif // ]

///////////////////////////////////////////////////////////////////////////////
/*
 * The purpose of this data structure reminds us on De Broglie–Bohm theory's
 * pilot wave (https://en.wikipedia.org/wiki/De_Broglie–Bohm_theory.) Hence
 * we called it "pilot".
 */
typedef const struct bg_pilot_hist_vmt bg_pilot_hist_vmt_t;
typedef struct bg_pilot_hist_leaf bg_pilot_hist_leaf_t;
typedef struct bg_pilot_hist_branch bg_pilot_hist_branch_t;
typedef struct bg_pilot_hist bg_pilot_hist_t;
typedef const struct bg_pilot_callback bg_pilot_callback_t;
typedef struct bg_pilot_client bg_pilot_client_t;
typedef struct bg_pilot bg_pilot_t;

///////////////////////////////////////////////////////////////////////////////
struct bg_pilot_hist_vmt {
#if defined (PBU_DEBUG) // [
  const char *id;
#endif // ]
  void (*destroy)(bg_pilot_hist_t *hist);
  const ffchar_t *(*first)(bg_pilot_hist_t *hist);
  const ffchar_t *(*next)(bg_pilot_hist_t *hist);
};

////////
struct bg_pilot_hist_leaf {
#if defined (_MSC_VER) // [
  // just to make msc happy ...
  int __noop;
#endif // ]
};

int bg_pilot_hist_leaf_create(bg_pilot_hist_t *hist);

////////
struct bg_pilot_hist_branch {
#if defined (_WIN32) // [
  HANDLE hFind;
  WIN32_FIND_DATAW e;
#else // ] [
  DIR *dir;
  struct dirent *e;
#endif // ]
};

#if defined (_WIN32) // [
int bg_pilot_hist_branch_create(bg_pilot_hist_t *hist, HANDLE hFind,
    WIN32_FIND_DATAW *e);
#else // ] [
int bg_pilot_hist_branch_create(bg_pilot_hist_t *hist, DIR *dir);
#endif // ]

////////
struct bg_pilot_hist {
  bg_pilot_hist_vmt_t *vmt;

  // common data are ceated (destroyed) in bg_pilot::push (bg_pilot::pop.) [
  ffchar_t *path;
  bg_pilot_t *pilot;
  void *data;
  // ]

  union {
    bg_pilot_hist_leaf_t leaf;
    bg_pilot_hist_branch_t branch;
  };
};

////////
struct bg_pilot_callback {
  struct {
    int (*enter)(bg_pilot_hist_t *hist, void *data);
    void (*leave)(bg_pilot_hist_t *hist, void *data);
  } leaf;

  struct {
    int (*enter)(bg_pilot_hist_t *hist, void *data);
    void (*leave)(bg_pilot_hist_t *hist, void *data);
  } branch;
};

////////
struct bg_pilot_client {
  bg_pilot_callback_t *cb;
  void *data;
};

////////
struct bg_pilot {
  bg_pilot_client_t client;
  bg_pilot_hist_t *min;
  bg_pilot_hist_t *nxt;
  bg_pilot_hist_t *max;
};

int bg_pilot_create(bg_pilot_t *pilot, size_t size, bg_pilot_callback_t *cb,
    void *data);
void bg_pilot_destroy(bg_pilot_t *pilot);

int bg_pilot_first(bg_pilot_t *pilot, const ffchar_t *path);
int bg_pilot_next(bg_pilot_t *pilot, int size);
int bg_pilot_loop(bg_pilot_t *pilot, const ffchar_t *path);
int bg_pilot_empty(const bg_pilot_t *pilot);

#if defined (__cplusplus) // [
}
#endif // ]
#endif // __BG_PILOT_H__ ]
