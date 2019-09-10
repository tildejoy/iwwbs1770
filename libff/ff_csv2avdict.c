/*
 * ff_csv2avdict.c
 *
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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

#if 0 && defined (strtok_r) // [
#undef strtok_r
#endif // ]

//#define DEBUG

#define PRIV_BUF_SIZE     1024
#define PRIV_FOLDER_CSV   FFL("folder.csv")

///////////////////////////////////////////////////////////////////////////////
#define PRIV_CH_SIZE 4

typedef struct priv priv_t;
typedef int (*priv_get_t)(priv_t *);

struct priv {
  char *buf;
  char *wp;
  char *mp;
  FILE *f;
  priv_get_t get;
  char ch[PRIV_CH_SIZE+1];
};

static priv_t *priv_init(priv_t *b, FILE *f);
static priv_t *priv_cleanup(priv_t *b);
static int priv_realloc(priv_t *b, size_t n);
static int priv_gets(priv_t *b);

#if defined (_WIN32) // {
static int priv_get_utf16le(priv_t *b);
static int priv_get_utf16be(priv_t *b);
#endif // }
static int priv_get_utf8(priv_t *b);
static int priv_get_ansi(priv_t *b);

///////////////////////////////////////////////////////////////////////////////
static priv_t *priv_init(priv_t *b, FILE *f)
{
#if defined (_WIN32) // {
  static const uint8_t utf16le[]={ 0xFF,0xFE,0x00 };
  static const uint8_t utf16be[]={ 0xFE,0xFF,0x00 };
#endif // }
  static const uint8_t utf8[]={ 0xEF,0xBB,0xBF,0x00 };

  long pos;
  const uint8_t *rp;
  uint8_t ch;

  memset(b,0,sizeof *b);

  if (NULL==(b->buf=malloc(PRIV_BUF_SIZE)))
    goto error;

  b->wp=b->buf;
  b->mp=b->buf+PRIV_BUF_SIZE;
  b->f=f;

  if ((pos=ftell(f)<-1l)) {
    DMESSAGE("getting file length");
    goto error;
  }

#if defined (_WIN32) // [
  // UTF-16 LE
  rp=utf16le;

  while (*rp) {
    if (1!=fread(&ch,1,1,f)||*rp++!=ch)
      goto utf16be;
  }

#if 0 && defined (PBU_DEBUG) // [
  fprintf(stderr,"UTF-16 LE\n");
#endif // ]

  if (0==(b->get=priv_get_utf16le)(b)) {
    DMESSAGE("getting utf16le");
    goto error;
  }

  return b;
utf16be:
  // UTF-16 BE
  if (0!=fseek(f,pos,SEEK_SET)) {
    DMESSAGE("seeking");
    goto error;
  }

  rp=utf16be;

  while (*rp) {
    if (1!=fread(&ch,1,1,f)||*rp++!=ch)
      goto utf8;
  }

#if 0 && defined (PBU_DEBUG) // [
  fprintf(stderr,"UTF-16 BE\n");
#endif // ]

  if (0==(b->get=priv_get_utf16be)(b)) {
    DMESSAGE("getting utf16be");
    goto error;
  }

  return b;
utf8:
  // UTF-8
  if (0!=fseek(f,pos,SEEK_SET)) {
    DMESSAGE("seeking");
    goto error;
  }
#endif // ]
  rp=utf8;

  while (*rp) {
    if (1!=fread(&ch,1,1,f)||*rp++!=ch)
      goto ansi;
  }

#if 0 && defined (PBU_DEBUG) // [
  fprintf(stderr,"UTF-8\n");
#endif // ]

  if (0==(b->get=priv_get_utf8)(b)) {
    DMESSAGE("getting utf8");
    goto error;
  }

  return b;
ansi:
  // ANSI
  if (0!=fseek(f,pos,SEEK_SET)) {
    DMESSAGE("seeking");
    goto error;
  }

#if 0 && defined (PBU_DEBUG)
  fprintf(stderr,"ANSI\n");
#endif

  if (0==(b->get=priv_get_ansi)(b)) {
    DMESSAGE("getting ansi");
    goto error;
  }

  return b;
error:
  priv_cleanup(b);

  return NULL;
}

static priv_t *priv_cleanup(priv_t *b)
{
  if (NULL!=b->buf)
    free(b->buf);

  return b;
}

static int priv_realloc(priv_t *b, size_t n)
{
  size_t size=b->mp-b->buf;
  size_t offs=b->wp-b->buf;
  char *tmp;

  if (size<offs+n) {
    do {
      size*=2;
    } while (size<offs+n);

    if (NULL==(tmp=realloc(b->buf,size))) {
      DMESSAGE("reallocationg");
      return -1;
    }

    b->buf=tmp;
    b->wp=b->buf+offs;
    b->mp=b->buf+size;
  }

  return 0;
}

#if 0 && defined (_WIN32) && defined (UNICODE) // [
static FILE *priv_fopen(const char *name, const wchar_t *wmode)
{
  int n1,n2;
  wchar_t *wname=NULL;
  FILE *f=NULL;

  n1=MultiByteToWideChar(
    CP_UTF8,      // __in       UINT CodePage,
    0,            // __in       DWORD dwFlags,
    name,         // __in       LPCSTR lpMultiByteStr,
    -1,           // __in       int cbMultiByte,
    NULL,         // __out_opt  LPWSTR lpWideCharStr,
    0             // __in       int cchWideChar
  );

  if (0==n1||NULL==(wname=malloc(n1*sizeof *wname)))
    goto cleanup;

  n2=MultiByteToWideChar(
    CP_UTF8,      // __in       UINT CodePage,
    0,            // __in       DWORD dwFlags,
    name,         // __in       LPCSTR lpMultiByteStr,
    -1,           // __in       int cbMultiByte,
    wname,        // __out_opt  LPWSTR lpWideCharStr,
    n1            // __in       int cchWideChar
  );

  if (n2!=n1)
    goto cleanup;

  f=_wfopen(wname,wmode);
cleanup:
  if (NULL!=wname)
    free(wname);

  return f;
}
#endif // ]

static int priv_get_ansi(priv_t *b)
{
  if (1!=fread(b->ch,1,1,b->f))
    goto error;

  b->ch[1]=0;

  return 1;
error:
  b->ch[0]=0;

  return 0;
}

// http://zaemis.blogspot.de/2011/06/reading-unicode-utf-8-by-hand-in-c.html
static int priv_get_utf8(priv_t *b)
{
  // mask values for bit pattern of first byte in multi-byte
  // UTF-8 sequences: 
  //   192 - 110xxxxx - for U+0080 to U+07FF 
  //   224 - 1110xxxx - for U+0800 to U+FFFF 
  //   240 - 11110xxx - for U+010000 to U+1FFFFF
  static unsigned short mask[] = {192, 224, 240}; 

  char *wp=b->ch;
  size_t n; 
  int i;

  // read first byte into buffer
  if (EOF==(i=getc(b->f))) {
    // we don't want to have an error message when EOF.
    //DMESSAGE("unexpected EOF");
    goto error;
  }

  *wp=(char)i;

  // check how many more bytes need to be read for character
  n = 0;

  while (n<3 && (*wp & mask[n]) == mask[n])
    ++n;

  ++wp;

  // read subsequent character bytes
  if (0<n) {
    if (n!=fread(wp,1,n,b->f)) {
      DMESSAGE("reading");
	    goto error;
    }

    wp+=n;
  }

  *wp=0;

  // return number of bytes read into buffer
  return 1+n;
error:
  b->ch[0]=0;

  return 0;
}

#if defined (_WIN32) /*&& defined (UNICODE)*/ // {
static int priv_get_utf16(priv_t *b, int (*read)(wchar_t *, FILE *))
{
  wchar_t wch;
  int n;

  // NOTE: function read() is an argument!
  if (0!=read(&wch,b->f)) {
    // we don't want to have an error message when EOF.
    //DMESSAGE("reading");
    goto error;
  }

  n=WideCharToMultiByte(
    CP_UTF8,        // __in       UINT CodePage
    0,              // __in       DWORD dwFlags
    &wch,           // __in       LPCWSTR lpWideCharStr
    1,              // __in       int cchWideChar
    b->ch,          // __out_opt  LPSTR lpMultiByteStr
    PRIV_CH_SIZE,   // __in       int cbMultiByte
    NULL,           // __in_opt   LPCSTR lpDefaultChar
    NULL            // __out_opt  LPBOOL lpUsedDefaultChar
  );

  if (PRIV_CH_SIZE<n) {
    DMESSAGE("converting");
    goto error;
  }

  b->ch[n]=0;

  return n;
error:
  b->ch[0]=0;

  return 0;
}

static int priv_read_utf16le(wchar_t *wch, FILE *f)
{
  if (1!=fread(wch,sizeof *wch,1,f)) {
    // we don't want to have an error message when EOF.
    //DMESSAGE("reading");
    return -1;
  }

  return 0;
}

static int priv_get_utf16le(priv_t *b)
{
  return priv_get_utf16(b,priv_read_utf16le);
}

static int priv_read_utf16be(wchar_t *wch, FILE *f)
{
  if (1!=fread(((char *)wch)+1,1,1,f)) {
    DMESSAGE("reading");
    return -1;
  }

  if (1!=fread((char *)wch,1,1,f)) {
    DMESSAGE("reading");
    return -1;
  }

  return 0;
}

static int priv_get_utf16be(priv_t *b)
{
  return priv_get_utf16(b,priv_read_utf16be);
}
#endif // }

static int priv_gets(priv_t *b)
{
  priv_get_t get=b->get;
  char *ch=b->ch;
  int n;

  b->wp=b->buf;

  for (;;) {
    if (0==*ch)
      goto end_line;
    else if (0==strcmp("\r",ch)) {
	    if (0==get(b))
        goto end_line;
      else if (0==strcmp("\n",ch))
        get(b);

      goto end_line;
    }
    else if (0==strcmp("\n",ch)) {
	    if (0==get(b))
        goto end_line;
      else if (0==strcmp("\r",ch))
        get(b);

      goto end_line;
    }

	  n=strlen(ch);

    if (0!=priv_realloc(b,n)) {
      DMESSAGE("reallocating");
      return -1;
    }

    memcpy(b->wp,ch,n);
	  b->wp+=n;
	
	  if (0==get(b))
	    goto end_line;

    continue;
  end_line:
    if (0!=priv_realloc(b,1)) {
      DMESSAGE("reallocating");
      return -1;
    }

    *b->wp++=0;
#if 0 && defined (PBU_DEBUG) // [
    fprintf(stderr,"\"%s\"\n",b->buf);
#endif // ]

    break;
  }

  return 0;
}

static int priv_loop(FILE *f, const char *name, char sep,
    AVDictionary **metadata)
{
  priv_t b;
  char tok[]={sep,0};
  char *head=NULL;
  char *rp=NULL;
  char *pp=NULL;
  char *hrp=NULL;
  char *hmp;
  char *np=NULL;
  int code=-1;

  (void)np;
  memset(&b,0,sizeof b);

  // initilialize parser.
  if (NULL==priv_init(&b,f)) {
    DMESSAGE("initializing");
    goto cleanup;
  }

  // read the header.
  if (0!=priv_gets(&b)) {
    DMESSAGE("initializing");
    goto cleanup;
  }

  if (NULL==(head=malloc(b.wp-b.buf))) {
    DMESSAGE("initializing");
    goto cleanup;
  }

  memcpy(head,b.buf,b.wp-b.buf);
  hmp=head+(b.wp-b.buf);

  for (rp=strtok_r(head,tok,&np);NULL!=rp;rp=strtok_r(NULL,tok,&np))
    ;

  if (0!=strcmp("file",head)) {
    DMESSAGE("field \"file\" undefined");
    goto cleanup;
  }

  do {
    if (0!=priv_gets(&b))
      goto cleanup;

    pp=b.buf;
    rp=strtok_r(b.buf,tok,&np);

    for (hrp=head;hrp<hmp&&NULL!=hrp&&NULL!=rp;hrp=hrp+strlen(hrp)+1) {
      if (NULL==rp||pp<rp)
        ++pp;
      else {
        if (0==strcmp("file",hrp)||0==*rp) {
          if (0!=strcmp(name,rp))
            goto next_line;
        }
        else if (av_dict_set(metadata,hrp,rp,0)<0) {
          DMESSAGE("setting av_dict");
          goto cleanup;
        }

        pp=rp+strlen(rp)+1;
        rp=strtok_r(NULL,tok,&np);
      }
    }

    code=0;
    break;
  next_line:
    ;
  } while (0!=*b.ch);
cleanup:
  if (NULL!=head)
    free(head);

  priv_cleanup(&b);

  return code;
}

static const char *priv_name(const char *s)
{
  const char *p=s;

  for (;;) {
    char *p1=strstr(p,"\\");
    char *p2=strstr(p,"/");
    char *p3=p1&&p2?(p2<p1?p1:p2):p1?p1:p2?p2:NULL;

    if (NULL==p3)
      break;

#if defined (_WIN32)
    p=CharNextA(p3);
#else
    p=p3+1;
#endif
  }

  return p;
}

#if defined (_WIN32) // [
static const wchar_t *priv_namew(const wchar_t *s)
{
  const wchar_t *p=s;

  for (;;) {
    wchar_t *p1=wcsstr(p,L"\\");
    wchar_t *p2=wcsstr(p,L"/");
    wchar_t *p3=p1&&p2?(p2<p1?p1:p2):p1?p1:p2?p2:NULL;

    if (NULL==p3)
      break;

#if defined (_WIN32)
    p=CharNextW(p3);
#else
    p=p3+1;
#endif
  }

  return p;
}

int ff_csv2avdict(const char *path, const wchar_t *pathw, char sep,
    AVDictionary **metadata, int erropen)
#else // ] [
int ff_csv2avdict(const char *path, char sep,
    AVDictionary **metadata, int erropen)
#endif // ]
{
  int len=0;
  const char *name=NULL;
#if defined (WIN32) // [
  const wchar_t *namew=NULL;
  wchar_t *csvw=NULL;
#else // ] [
  char *csv=NULL;
#endif // ]
  FILE *f=NULL;
  int code=-1;

  name=priv_name(path);
#if defined (WIN32) // [
  namew=priv_namew(pathw);
  len=namew-pathw;
  csvw=malloc((len+wcslen(PRIV_FOLDER_CSV)+1)*sizeof csvw[0]);

  if (!csvw) {
    DMESSAGE("allocating");
    goto cleanup;
  }

  memcpy(csvw,pathw,len*sizeof csvw[0]);
  csvw[len]=0;
  wcscpy(csvw+len,PRIV_FOLDER_CSV);
  f=_wfopen(csvw,L"rb");

  if (!f) {
    if (erropen)
      DVMESSAGEW("opening %s",csvw);
    else
      code=0;

    goto cleanup;
  }
#else // ] [
  len=name-path;
  csv=malloc((len+strlen(PRIV_FOLDER_CSV)+1)*sizeof csv[0]);

  if (!csv) {
    DMESSAGE("allocating");
    goto cleanup;
  }

  memcpy(csv,path,len*sizeof csv[0]);
  strcpy(csv+len,PRIV_FOLDER_CSV);
  f=fopen(csv,"rb");

  if (!f) {
    if (erropen)
      DVMESSAGE("opening %s",csv);
    else
      code=0;

    goto cleanup;
  }
#endif // ]

  code=priv_loop(f,name,sep,metadata);
cleanup:
  if (NULL!=f)
    fclose(f);

#if defined (_WIN32) // [
  if (NULL!=csvw)
    free(csvw);
#else // ] [
  if (NULL!=csv)
    free(csv);
#endif // ]

  return code;
}

#if 0 // [
///////////////////////////////////////////////////////////////////////////////
#define FN "Joe_Walsh_-_Rocky_Mountain_Way_-_Vintage_Live_Footage_1972.flv"
//#define FN "Joe_Walsh_-_Turn_To_Stone_-_Vintage_Live_Footage_1972.flv"

int main(int argc, char **argv)
{
  AVDictionary *dict=NULL;
  AVDictionaryEntry *tag=NULL;

  if (0!=priv_csv2dict(argv[1],FN,'\t',&dict))
    goto cleanup;

  while (NULL!=(tag=av_dict_get(dict,"",tag,AV_DICT_IGNORE_SUFFIX)))
    printf("%s=%s\n",tag->key,tag->value);
cleanup:
  av_dict_free(&dict);

  return 0;
}

int main1(int argc, char **argv)
{
  FILE *f=NULL;
  priv_t b;
  char *head=NULL;
  char *rp=NULL;
  char *pp=NULL;
  char *hrp=NULL;
  char *hmp;
#if ! defined (_WIN32)
  char *np=NULL;
#endif

  memset(&b,0,sizeof b);

  // open input file.
  if (NULL==(f=fopen(argv[1],"rb")))
    goto cleanup;

  // initilialize parser.
  if (NULL==priv_init(&b,f))
    goto cleanup;

  // read the header.
  if (0!=priv_gets(&b))
    goto cleanup;

  if (NULL==(head=malloc(b.wp-b.buf)))
    goto cleanup;

  memcpy(head,b.buf,b.wp-b.buf);
  hmp=head+(b.wp-b.buf);

#if defined (_WIN32)
  for (rp=strtok(head,"\t");NULL!=rp;rp=strtok(NULL,"\t"))
#else
  for (rp=strtok_r(head,"\t",&np);NULL!=rp;rp=strtok_r(NULL,"\t",&np))
#endif
    //printf("%s\n",rp);
    ;

  do {
    if (0!=priv_gets(&b))
      goto cleanup;

    pp=b.buf;
    rp=strtok(b.buf,"\t");

    for (hrp=head;hrp<hmp&&NULL!=hrp&&NULL!=rp;hrp=hrp+strlen(hrp)+1) {
      if (NULL==rp||pp<rp)
        ++pp;
      else {
        printf("%s: %s\n",hrp,rp);
        pp=rp+strlen(rp)+1;
        rp=strtok(NULL,"\t");
      }
    }

    printf("\n");
  } while (EOF!=b.ch);
cleanup:
  if (NULL!=head)
    free(head);

  priv_cleanup(&b);

  if (NULL!=f)
    fclose(f);

  return 0;
}
#endif // ]
