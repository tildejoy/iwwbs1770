/*
 * bg_pivot_leaf.c
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
static bg_pilot_hist_vmt_t bg_pilot_hist_leaf_vmt;

///////////////////////////////////////////////////////////////////////////////
int bg_pilot_hist_leaf_create(bg_pilot_hist_t *hist)
{
  bg_pilot_client_t *client=&hist->pilot->client;

  /////////////////////////////////////////////////////////////////////////////
  hist->vmt=&bg_pilot_hist_leaf_vmt;

  /////////////////////////////////////////////////////////////////////////////
  if (client->cb&&client->cb->leaf.enter) {
    if (client->cb->leaf.enter(hist,client->data)<0) {
      DMESSAGE("entering leaf");
      goto e_enter;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_enter:
  return -1;
}

static void bg_pilot_hist_leaf_destroy(bg_pilot_hist_t *hist FFUNUSED)
{
  bg_pilot_client_t *client=&hist->pilot->client;

  if (client->cb&&client->cb->leaf.leave)
    client->cb->leaf.leave(hist,client->data);
}

///////////////////////////////////////////////////////////////////////////////
static const ffchar_t *bg_pilot_hist_leaf_first(bg_pilot_hist_t *hist FFUNUSED)
{
  return NULL;
}

static const ffchar_t *bg_pilot_hist_leaf_next(bg_pilot_hist_t *hist FFUNUSED)
{
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
static bg_pilot_hist_vmt_t bg_pilot_hist_leaf_vmt={
#if defined (PBU_DEBUG) // [
  .id="leaf",
#endif // ]
  .destroy=bg_pilot_hist_leaf_destroy,
  .first=bg_pilot_hist_leaf_first,
  .next=bg_pilot_hist_leaf_next,
};
