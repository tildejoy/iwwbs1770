/*
 * pbutil_priv.h
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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
#ifndef __PBUTIL_PRIV_H__
#define __PBUTIL_PRIV_H__ // {
#include <pbutil.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define DMARKLN()                   PBU_DMARKLN()
#define DWRITELN(cs)                PBU_DWRITELN(cs)
#define DVWRITELN(format,...)       PBU_DVWRITELN(format,__VA_ARGS__)
#if defined (_WIN32) // [
#define DWRITELNW(cs)               PBU_DWRITELNW(cs)
#define DVWRITELNW(format,...)      PBU_DVWRITELNW(format,__VA_ARGS__)
#endif // ]
#define DPUTS(cs)                   PBU_DPUTS(cs)
#define DPRINTF(cs,...)             PBU_DPRINTF(cs,__VA_ARGS__)
#define DPUTWS(ws)                  PBU_DPUTWS(ws)
#define DWPRINTF(ws,...)            PBU_DWPRINTF(ws,__VA_ARGS__)
#define DERROR(x,y)                 PBU_DERROR(x,y)
#define DMESSAGE(m)                 PBU_DMESSAGE(m)
#define DVMESSAGE(m,...)            PBU_DVMESSAGE(m,__VA_ARGS__)
#define DVMESSAGEW(m,...)           PBU_DVMESSAGEW(m,__VA_ARGS__)
#define DWARNING(m)                 PBU_DWARNING(m)
#define DVWARNING(m,...)            PBU_DVWARNING(m,__VA_ARGS__)
#define DVWARNINGW(m,...)           PBU_DVWARNINGW(m,__VA_ARGS__)

#define DDPUTS(debug,cs)            PBU_DDPUTS(debug,cs)
#define DDPRINTF(debug,cs,...)      PBU_DDPRINTF(debug,cs,__VA_ARGS__)
#define DDPUTWS(debug,ws)           PBU_DDPUTWS(debug,ws)
#define DDWPRINTF(debug,ws,...)     PBU_DDWPRINTF(debug,ws,__VA_ARGS__)
#define DDERROR(debug,x,y)          PBU_DDERROR(debug,x,y)
#define DDMESSAGE(debug,m)          PBU_DDMESSAGE(debug,m)
#define DDVMESSAGE(debug,m,...)     PBU_DDVMESSAGE(debug,m,__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
#define TRACE_PUSH()                PBU_TRACE_PUSH()
#define TRACE_POP()                 PBU_TRACE_POP()
#define HEAP_PRINT()                PBU_HEAP_PRINT()
#if defined (_WIN32) // {
#define _WCSDUP(str)                _PBU_WCSDUP(str)
#endif // }
#define STRDUP(str)                 PBU_STRDUP(str)
#define MALLOC(size)                PBU_MALLOC(size)
#define CALLOC(num,size)            PBU_CALLOC(num,size)
#define REALLOC(ptr,size)           PBU_REALLOC(ptr,size)
#define FREE(ptr)                   PBU_FREE(ptr)

///////////////////////////////////////////////////////////////////////////////
#define MAXOF(T)                    PBU_MAXOF(T)

///////////////////////////////////////////////////////////////////////////////
#define LIST_APPEND(l,n)            PBU_LIST_APPEND(l,n)
#define LIST_NEXT(n,l)              PBU_LIST_NEXT(n,l)
#define LIST_FOREACH(n,l)           PBU_LIST_FOREACH(n,l)

///////////////////////////////////////////////////////////////////////////////
typedef pbu_list_t list_t;

#ifdef __cpluplus
}
#endif
#endif // }
