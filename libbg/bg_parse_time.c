/*
 * bg_parse_time.c
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
#if ! defined (_WIN32) // [
#include <ctype.h>
#endif // ]
#include <bg.h>

// parse timestamp in AV_TIME_BASE units.
int64_t bg_parse_time(const ffchar_t *s)
{
  enum { BASE=AV_TIME_BASE };
  int64_t ts=0ll;
  int64_t num=0ll;
  int64_t den=1ll;
  int ncolon=0;

  for (;*s;++s) {
    if (FFL(':'==*s)) {
      if (2==ncolon) {
        DVMESSAGE("more than %d colons in timestamp not supported",ncolon);
        goto exit;
      }

      ++ncolon;
      ts*=60ll;
    }
    else if (FFISDIGIT(*s)) {
      ts*=10ll;
      ts+=*s-FFL('0');
    }
    else {
      DVMESSAGE("unexpected character '\\x%x' in timestamp",*s);
      goto exit;
    }
  }

  if (FFL('.'==*s)) {
    for (++s;*s;++s) {
      if (FFISDIGIT(*s)) {
        num*=10ll;
        num+=*s-FFL('0');
        den*=10ll;
      }
      else {
        DVMESSAGE("unexpected character '\\x%x' in timestamp",*s);
        goto exit;
      }
    }
  }
exit:
  ts*=BASE;
  ts+=BASE*num/den;

  return ts;
}
