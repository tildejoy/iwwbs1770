##
 # configure.ac.template
 # Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
 #
 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public
 # License as published by the Free Software Foundation; either
 # version 2.0 of the License, or (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # General Public License for more details.
 #
 # You should have received a copy of the GNU General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 # MA  02110-1301  USA
 ##
# http://www.freesoftwaremagazine.com/books/autotools_a_guide_to_autoconf_automake_libtool
# https://autotools.io/autoconf/arguments.html
# http://www.gnu.org/software/libidn/manual/html_node/Autoconf-tests.html
# http://www.clearchain.com/blog/posts/autotools
#
# AC_ARG_WITH (option-name, help-string, action-if-present, action-if-not-present)
# AC_CHECK_HEADER (header-file, [action-if-found], [action-if-not-found], [includes = `default-includes'])
# AC_CHECK_LIB (library, function, [action-if-found], [action-if-not-found], [other-libraries])

AC_INIT([iwwbs1770], [0.6.1-beta-1], [tilde@ultros.pro], [], [http://iww.org.au/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR()
AC_PROG_CC
AC_PROG_RANLIB

# test for win32 {
AC_MSG_CHECKING([for win32])
tmp=`mktemp`
cat << _ACEOF | gcc -xc -o "$tmp" -
#include <stdio.h>

int main()
{ 
#if defined (_WIN32) // {
  puts("yes");
#else // } {
  puts("no");
#endif // }
  return 0;
}
_ACEOF

win32="`$tmp`"
rm -f "$tmp"
AM_CONDITIONAL([WIN32], [test x$win32 = xyes])
AC_MSG_RESULT($win32)
# }

LIBS_INITIAL=${LIBS}

# test for FFmpeg [
unset withval
AC_ARG_WITH(ffmpeg, AS_HELP_STRING([--with-ffmpeg=[DIR]], [path to FFmpeg installation]),
    ffmpeg=$withval, ffmpeg=yes)
if test "x$ffmpeg" != "xno"; then # [
  if test "x$ffmpeg" != "xyes"; then # [
    LDFLAGS="${LDFLAGS} -L$ffmpeg/lib"
    CPPFLAGS="${CPPFLAGS} -I$ffmpeg/include"
    if test "x$win32" = "xno"; then # [
      export LD_LIBRARY_PATH="$ffmpeg/lib:${LD_LIBRARY_PATH}"
    else # ] [
      export PATH="$ffmpeg/bin:${PATH}"
    fi # ]
    FFMPEG_PREFIX="$ffmpeg"
    AC_SUBST(FFMPEG_PREFIX)
  fi # ]
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavutil/avutil.h,
    AC_CHECK_LIB(avutil, avutil_version, [ffmpeg=yes LIBS="${LIBS} -lavutil"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswscale/swscale.h,
    AC_CHECK_LIB(swscale, swscale_version, [ffmpeg=yes LIBS="${LIBS} -lswscale"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavcodec/avcodec.h,
    AC_CHECK_LIB(avcodec, avcodec_version, [ffmpeg=yes LIBS="${LIBS} -lavcodec"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavformat/avformat.h,
    AC_CHECK_LIB(avformat, avformat_version, [ffmpeg=yes LIBS="${LIBS} -lavformat"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswresample/swresample.h,
    AC_CHECK_LIB(swresample, swresample_version, [ffmpeg=yes LIBS="${LIBS} -lswresample"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswscale/swscale.h,
    AC_CHECK_LIB(swscale, swscale_version, [ffmpeg=yes LIBS="${LIBS} -lswscale"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libpostproc/postprocess.h,
    AC_CHECK_LIB(postproc, postproc_version, [ffmpeg=yes LIBS="${LIBS} -lpostproc"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavfilter/avfilter.h,
    AC_CHECK_LIB(avfilter, avfilter_version, [ffmpeg=yes LIBS="${LIBS} -lavfilter"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" == "xno" ; then # [
  AC_MSG_ERROR([FFmpeg not found])
fi # ]
# test for FFmpeg ]

# define avutil major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavutil/avutil.h>

int main() { printf("%u",avutil_version()>>16); return 0; }
_ACEOF

FF_AVUTIL_V="`$tmp`"
AC_SUBST(FF_AVUTIL_V)
AC_DEFINE_UNQUOTED([FF_AVUTIL_V], ["$FF_AVUTIL_V"], [Define to libavutil major version.])
rm -f "$tmp"
# define avutil major version ]

# define avcodec major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavcodec/avcodec.h>

int main() { printf("%u",avcodec_version()>>16); return 0; }
_ACEOF

FF_AVCODEC_V="`$tmp`"
AC_SUBST(FF_AVCODEC_V)
AC_DEFINE_UNQUOTED([FF_AVCODEC_V], ["$FF_AVCODEC_V"], [Define to libavcodec major version.])
rm -f "$tmp"
# define avcodec major version ]

# define avformat major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavformat/avformat.h>

int main() { printf("%u",avformat_version()>>16); return 0; }
_ACEOF

FF_AVFORMAT_V="`$tmp`"
AC_SUBST(FF_AVFORMAT_V)
AC_DEFINE_UNQUOTED([FF_AVFORMAT_V], ["$FF_AVFORMAT_V"], [Define to libavformat major version.])
rm -f "$tmp"
# define avformat major version ]

# define swresample major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libswresample/swresample.h>

int main() { printf("%u",swresample_version()>>16); return 0; }
_ACEOF

FF_SWRESAMPLE_V="`$tmp`"
AC_SUBST(FF_SWRESAMPLE_V)
AC_DEFINE_UNQUOTED([FF_SWRESAMPLE_V], ["$FF_SWRESAMPLE_V"], [Define to libswresample major version.])
rm -f "$tmp"
# define swresample major version ]

# define swscale major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libswscale/swscale.h>

int main() { printf("%u",swscale_version()>>16); return 0; }
_ACEOF

FF_SWSCALE_V="`$tmp`"
AC_SUBST(FF_SWSCALE_V)
AC_DEFINE_UNQUOTED([FF_SWSCALE_V], ["$FF_SWSCALE_V"], [Define to libswscale major version.])
rm -f "$tmp"
# define swscale major version ]

# define postproc major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libpostproc/postprocess.h>

int main() { printf("%u",postproc_version()>>16); return 0; }
_ACEOF

FF_POSTPROC_V="`$tmp`"
AC_SUBST(FF_POSTPROC_V)
AC_DEFINE_UNQUOTED([FF_POSTPROC_V], ["$FF_POSTPROC_V"], [Define to libswscale major version.])
rm -f "$tmp"
# define postproc major version ]

# define avfilter major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavfilter/avfilter.h>

int main() { printf("%u",avfilter_version()>>16); return 0; }
_ACEOF

FF_AVFILTER_V="`$tmp`"
AC_SUBST(FF_AVFILTER_V)
AC_DEFINE_UNQUOTED([FF_AVFILTER_V], ["$FF_AVFILTER_V"], [Define to libavfilter major version.])
rm -f "$tmp"
# define avfilter major version ]

# test for dynload [
unset withval
# AC_ARG_ENABLE(dynload, AS_HELP_STRING([--enable-dynload], [load FFmpeg and SoX dynamically]),
#     dynload=yes, dynload=no)
AC_ARG_ENABLE(dynload, AS_HELP_STRING([--enable-dynload], [load FFmpeg dynamically]),
    dynload=yes, dynload=no)
AM_CONDITIONAL([FF_DYNLOAD], [test x$dynload = xyes])
if test "x$dynload" = "xyes"; then # [
  # revert LIBS to it's initial state
  # why??? because when loading ffmpeg libs dynamically we don't want to link
  # them! instead we have a special module doing the necessary forwarding.
  LIBS=${LIBS_INITIAL}
  AC_DEFINE([HAVE_FF_DYNLOAD], [1],
      [Define to 1 if FFmpeg should be loaded dynamically.])
  # test for dl [
  if test "xyes" != "x$win32"; then # [
    unset withval
    AC_ARG_WITH(dl, AS_HELP_STRING([--with-dl=[DIR]],
        [path to libdl installation]), dl=$withval, dl=yes)
    if test "x$dl" != "xno"; then # [
      if test "x$dl" != "xyes"; then # [
        LDFLAGS="${LDFLAGS} -L$dl/lib"
        CPPFLAGS="${CPPFLAGS} -I$dl/include"
      fi # ]
    fi # ]
    if test "x$dl" != "xno"; then # [
      AC_CHECK_HEADER(dlfcn.h,
        AC_CHECK_LIB(dl, dlopen, [dl=yes LIBS="${LIBS} -ldl"], dl=no),
        dl=no)
    fi # ]
    if test "x$dl" == "xno" ; then # [
      AC_MSG_ERROR([libdl not found])
    fi # ]
  fi # ]
  # test for dl ]
  AC_MSG_NOTICE([loading FFmpeg dynamically])
else # ] [
  AC_MSG_NOTICE([linking FFmpeg])
fi # ]
# test for dynload ]

if test "xno" != "x$win32"; then # [
  unset withval
  AC_ARG_WITH(getoptw,
      AS_HELP_STRING([--with-getoptw=[DIR]],[path to getoptW installation]),
      getoptw=$withval, getoptw=yes)
  if test "x$getoptw" != "xno"; then # [
    if test "x$getoptw" != "xyes"; then # [
      LDFLAGS="${LDFLAGS} -L$getoptw/lib"
      CPPFLAGS="${CPPFLAGS} -I$getoptw/include"
#      if test "$win32" = "xno"; then # [
#        export LD_LIBRARY_PATH="$getoptw/lib:${LD_LIBRARY_PATH}"
#      else # ] [
#        export PATH="$getoptw/bin:${PATH}"
#      fi # ]
      GETOPTW_PREFIX="$getoptw"
      AC_SUBST(GETOPTW_PREFIX)
    fi # ]
  fi # ]
  if test "x$getoptw" != "xno"; then # [
    AC_CHECK_HEADER(getoptW.h,
      AC_CHECK_LIB(getoptW,
          getoptW_long,
          [getoptw=yes LIBS="${LIBS} -lgetoptW"],
          getoptw=no),
      getoptw=no,
         [[#ifdef HAVE_GETOPTW_H
         # include <getoptW.h>
         #endif
         ]])
  else # ] [
    AC_MSG_ERROR([getoptW not found])
  fi # ]
fi # ]

# dealing with shared objects [
if test "x$win32" = "xyes"; then # [
  SODIR='bin'
  AVUTIL_SO=avutil-$FF_AVUTIL_V.dll
  SWSCALE_SO=swscale-$FF_SWSCALE_V.dll
  AVCODEC_SO=avcodec-$FF_AVCODEC_V.dll
  AVFORMAT_SO=avformat-$FF_AVFORMAT_V.dll
  SWRESAMPLE_SO=swresample-$FF_SWRESAMPLE_V.dll
  SWSCALE_SO=swscale-$FF_SWSCALE_V.dll
  POSTPROC_SO=postproc-$FF_POSTPROC_V.dll
  AVFILTER_SO=avfilter-$FF_AVFILTER_V.dll
  # not needed:
  #LDFLAGS=" ${LDFLAGS} -static-libgcc -static-libstdc++"
  # -mwindows for linking a windows application.
  # -municode for linking wmain() instead of main,() i.e. an unicode
  #   application. Just -municode is needed:
  #LDFLAGS=" ${LDFLAGS} -mwindows -municode"
  LDFLAGS=" ${LDFLAGS} -municode"
else # ] [
  SODIR='lib'
  AVUTIL_SO=libavutil.so.$FF_AVUTIL_V
  SWSCALE_SO=libswscale.so.$FF_SWSCALE_V
  AVCODEC_SO=libavcodec.so.$FF_AVCODEC_V
  AVFORMAT_SO=libavformat.so.$FF_AVFORMAT_V
  SWRESAMPLE_SO=libswresample.so.$FF_SWRESAMPLE_V
  SWSCALE_SO=libswscale.so.$FF_SWSCALE_V
  POSTPROC_SO=libpostproc.so.$FF_POSTPROC_V
  AVFILTER_SO=libavfilter.so.$FF_AVFILTER_V
	if test "x$dynload" = "yes"; then # [
	  LIBS="${LIBS} -ld"
	else # ] [
	  LIBS="${LIBS} -lm"
	fi # ]
fi # ]

AC_SUBST(SODIR)
AC_SUBST(AVUTIL_SO)
AC_SUBST(SWRESAMPLE_SO)
AC_SUBST(SWSCALE_SO)
AC_SUBST(AVCODEC_SO)
AC_SUBST(AVFORMAT_SO)
AC_SUBST(POSTPROC_SO)
AC_SUBST(AVFILTER_SO)
AC_SUBST(IWWBS1770_LDFLAGS)
# dealing with shared objects ]

CFLAGS="-Werror ${CFLAGS}"
CFLAGS="-Wall ${CFLAGS}"
CFLAGS="-Wformat ${CFLAGS}"
CFLAGS="-Wpointer-arith ${CFLAGS}"
CFLAGS="-Wcast-align ${CFLAGS}"
CFLAGS="-Wcast-qual ${CFLAGS}"
CFLAGS="-Wreturn-type ${CFLAGS}"
CFLAGS="-Wextra ${CFLAGS}"
CFLAGS="-Wstrict-prototypes ${CFLAGS}"

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  libpbutil/Makefile
  lib1770-2/Makefile
  libff/Makefile
  libbg/Makefile
])
AC_OUTPUT
