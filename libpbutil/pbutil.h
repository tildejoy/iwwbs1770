/*
 * pbutil.h
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
#ifndef __PBUTIL_H__
#define __PBUTIL_H__ // {
#if defined (HAVE_CONFIG_H) // {
#include <config.h>
#endif // }
#if defined (_WIN32) // {
#include <windows.h>
#endif // }
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//#define PBU_DEBUG
#define PBU_LIST_RING

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // {
#define PBU_WIDEN2(x) L ## x
#define PBU_WIDEN(x) PBU_WIDEN2(x)
#define __func__ __FUNCTION__
#endif // }

#define _PBU_STR(s) #s
#define PBU_STR(s) _PBU_STR(s)

///////////////////////////////////////////////////////////////////////////////
//#define PBU_MALLOC_DEBUG
#if defined (PBU_MALLOC_DEBUG) // {
#define PBU_TRACE_PUSH() \
    pbu_trace_push(__func__)
#define PBU_TRACE_POP() \
    pbu_trace_pop()
#define PBU_HEAP_PRINT() \
    pbu_heap_print()
#if defined (_WIN32) // {
#define _PBU_WCSDUP(str) \
    _pbu_wcsdup(__func__,pbu_basename(__FILE__),__LINE__,str)
#endif // }
#define PBU_STRDUP(str) \
    pbu_strdup(__func__,pbu_basename(__FILE__),__LINE__,str)
#define PBU_MALLOC(size) \
    pbu_malloc(__func__,pbu_basename(__FILE__),__LINE__,size)
#define PBU_CALLOC(num,size) \
    pbu_calloc(__func__,pbu_basename(__FILE__),__LINE__,num,size)
#define PBU_REALLOC(ptr,size) \
    pbu_realloc(__func__,pbu_basename(__FILE__),__LINE__,ptr,size)
#define PBU_FREE(ptr) \
    pbu_free(__func__,pbu_basename(__FILE__),__LINE__,ptr)
#define PBU_FREE_PTR            pbu_free1
#else // } {
#define PBU_TRACE_PUSH()
#define PBU_TRACE_POP()
#define PBU_HEAP_PRINT()
#if defined (_WIN32) // {
#define _PBU_WCSDUP(str)        _wcsdup(str)
#endif // }
#define PBU_STRDUP(str)         strdup(str)
#define PBU_MALLOC(size)        malloc(size)
#define PBU_CALLOC(num,size)    calloc(num,size)
#define PBU_REALLOC(ptr,size)   realloc(ptr,size)
#define PBU_FREE(ptr)           free(ptr)
#define PBU_FREE_PTR            free
#endif // }

///////////////////////////////////////////////////////////////////////////////
#define PBU_MIN(x,y) ((x)<(y)?(x):(y))

#define PBU_MAXOF(T) \
  (~(~0ull<<(((sizeof (T))<<3)-1)))

#if defined (_WIN32) // {
#define wcstok_r(str,delim,saveptr) pbu_wcstok_r(str,delim,saveptr)
#define strtok_r(str,delim,saveptr) pbu_strtok_r(str,delim,saveptr)
#endif // }

#if defined (_MSC_VER) // {
#define strcasecmp(s1,s2)       stricmp(s1,s2)
//#define round(x)                floor((x)+0.5)
#define S_ISDIR(st_mode)        (_S_IFDIR&(st_mode))
#define S_ISREG(st_mode)        (_S_IFREG&(st_mode))
#endif // }

///////////////////////////////////////////////////////////////////////////////
#define PBU_ERROR(m) "Error: " m "."
#define PBU_WARNING(m) "Warning: " m "."
#define PBU_DMESSAGE(m) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,PBU_ERROR(m),NULL)
#define PBU_DVMESSAGE(m,...) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,PBU_ERROR(m),__VA_ARGS__)
#if defined (_WIN32) // [
#define PBU_ERRORW(m) L"Error: " m L"."
#define PBU_DMESSAGEW(m) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,PBU_ERRORW(m),NULL)
#define PBU_DVMESSAGEW(m,...) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,PBU_ERRORW(m),__VA_ARGS__)
#endif // ]
#define PBU_DWARNING(m) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,PBU_WARNING(m),NULL)
#define PBU_DVWARNING(m,...) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,PBU_WARNING(m),__VA_ARGS__)
#if defined (_WIN32) // [
#define PBU_WARNINGW(m) L"Warning: " m L"."
#define PBU_DWARNINGW(m) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,PBU_WARNINGW(m),NULL)
#define PBU_DVWARNINGW(m,...) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,PBU_WARNINGW(m),\
        __VA_ARGS__)
#endif // ]
#if defined (PBU_DEBUG) // [
  #define PBU_DMARKLN() \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,NULL)
  #define PBU_DWRITELN(cs) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,cs)
  #define PBU_DVWRITELN(format, ...) \
    pbu_vwritelna(stderr,__FILE__,__LINE__,__func__,format,__VA_ARGS__)
#if defined (_WIN32) // [
  #define PBU_DWRITELNW(cs) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,cs)
  #define PBU_DVWRITELNW(format, ...) \
    pbu_vwritelnw(stderr,__FILE__,__LINE__,__func__,format,__VA_ARGS__)
#endif // ]
  #define PBU_DPUTS(cs) \
      fputs(cs,stderr)
  #define PBU_DPRINTF(cs,...) \
      fprintf(stderr,cs,__VA_ARGS__)
  #define PBU_DPUTWS(ws) \
      fputws(ws,stderr)
  #define PBU_DWPRINTF(ws,...) \
      fwprintf(stderr,ws,__VA_ARGS__)
  #define PBU_DERROR(x,y) \
      ((void)((x)==(y)&&fputs(#x "\n",stderr)))
  #define PBU_DDPUTS(debug,cs) \
      ((debug)&&PBU_DPUTS(cs))
  #define PBU_DDPRINTF(debug,cs,...) \
      ((debug)&&PBU_DPRINTF(cs,__VA_ARGS__))
  #define PBU_DDPUTWS(debug,ws) \
      ((debug)&&DPUTWS(ws))
  #define PBU_DDWPRINTF(debug,ws,...) \
      ((debug)&&PBU_DWPRINTF(ws,__VA_ARGS__))
  #define PBU_DDERROR(debug,x,y) \
      ((debug)&&PBU_DERROR(x,y))
  #define PBU_DDMESSAGE(debug,m) \
      ((debug)&&PBU_DMESSAGE(m))
  #define PBU_DDVDMESSAGE(debug,m,...) \
      ((debug)&&PBU_DVMESSAGE(m,__VA_ARGS__))
#else // ] [
  #define PBU_DMARKLN()
  #define PBU_DWRITELN(cs)
  #define PBU_DVWRITELN(format, ...)
#if defined (_WIN32) // [
  #define PBU_DWRITELNW(cs)
  #define PBU_DVWRITELNW(format, ...)
#endif // ]
  #define PBU_DPUTS(cs)
  #define PBU_DPRINTF(cs,...)
  #define PBU_DPUTWS(ws)
  #define PBU_DWPRINTF(ws,...)
  #if defined (PBU_MESSAGE) // [
    #define PBU_DERROR(debug,x,y) \
        ((void)((x)==(y)&&pbu_message(#x ": \"%s\" (%d).", \
            pbu_basename(__FILE__),__LINE__)))
  #else // ] [
    #define PBU_DERROR(x,y)
  #endif // ]
#if 0 // [
  #define PBU_DMESSAGE(m)
  #define PBU_DVMESSAGE(m,...)
#if defined (_WIN32) // [
  #define PBU_DMESSAGEW(m)
  #define PBU_DVMESSAGEW(m,...)
#endif // ]
#endif // ]
  ////
  #define PBU_DDPUTS(debug,cs)
  #define PBU_DDPRINTF(debug,cs,...)
  #define PBU_DDPUTWS(debug,ws)
  #define PBU_DDWPRINTF(debug,ws,...)
  #if defined (PBU_MESSAGE) // [
    #define PBU_DDERROR(debug,x,y) \
      ((debug)&&PBU_DERROR(x,y)
  #else // ] [
    #define PBU_DDERROR(debug,x,y)
  #endif // ]
  #define PBU_DDMESSAGE(debug,m)
  #define PBU_DDMESSAGEV(debug,m,...)
#endif // ]

///////////////////////////////////////////////////////////////////////////////
typedef struct pbu_list pbu_list_t;

///////////////////////////////////////////////////////////////////////////////
#if defined (PBU_MALLOC_DEBUG) // {
void pbu_trace_indent(void);
void pbu_trace_puts(const char *str);
void pbu_trace_push(const char *func);
void pbu_trace_pop(void);
void pbu_heap_print(void);
#if defined (_WIN32) // {
wchar_t *_pbu_wcsdup(const char *func, const char *file, int line,
    const wchar_t *str);
#endif // }
char *pbu_strdup(const char *func, const char *file, int line,
    const char *str);
void *pbu_malloc(const char *func, const char *file, int line, size_t size);
void *pbu_calloc(const char *func, const char *file, int line, size_t num,
    size_t size);
void *pbu_realloc(const char *func, const char *file, int line, void *ptr,
    size_t size);
void pbu_free1(void *ptr);
void pbu_free(const char *func, const char *file, int line, void *ptr);
#endif // }

//#define PBU_BASENAME_UNICODE
#if defined (_WIN32) // [
HANDLE pbu_msvcrt(void);
wchar_t *pbu_wcstok_r(wchar_t *str, const wchar_t *delim, wchar_t **saveptr);
char *pbu_strtok_r(char *str, const char *delim, char **saveptr);
int pbu_copy_file(const wchar_t *src, const wchar_t *dst);
wchar_t *pbu_s2w(const char *s);
char *pbu_w2s(const wchar_t *w);
const char *pbu_message(const char *format, ...);
const wchar_t *pbu_wbasename(const wchar_t *wpath);
void pbu_vwritelnw(FILE *f, const char *path, int line, const char *func,
    const wchar_t *format, ...);
#else // ] [
int pbu_copy_file(const char *src, const char *dst);
#endif // ]
void pbu_vwritelna(FILE *f, const char *path, int line, const char *func,
    const char *format, ...);
int pbu_same_file(const char *path1, const char *path2);
char *pbu_extend_path(const char *dirname, const char *basename);
const char *pbu_ext(const char *path);
const char *pbu_basename(const char *path);
void pbu_mkdir(char *path);

/// list //////////////////////////////////////////////////////////////////////
#define PBU_LIST_APPEND(l,n) \
  pbu_list_append(&(l),&(n),sizeof (n))
#define PBU_LIST_NEXT(n,l) \
  (*(n)=(*(n)==NULL||(l)==(*(n))->next?NULL:(*(n))->next))
#define PBU_LIST_FOREACH(n,l) \
  for (*(n)=(l);NULL!=*(n);PBU_LIST_NEXT(n,l))

struct pbu_list {
#define PBU_LIST_MEM(T) \
  T *prev; \
  T *next;
  PBU_LIST_MEM(pbu_list_t)
};

void *pbu_list_create(void *node);
int pbu_list_append(void *head, void *node, size_t size);
#if defined (PBU_LIST_RING) // [
void *pbu_list_remove_link(void *head, void *node);
#endif // ]
void pbu_list_free_full(void *head, void *free_func);
void pbu_list_free(void *head);

#ifdef __cpluplus
}
#endif
#endif // }
