#AM_CFLAGS=
#AM_LDFLAGS=
#
#AM_CFLAGS+=-Werror
#AM_CFLAGS+=-Wall
#AM_CFLAGS+=-Wformat
#AM_CFLAGS+=-Wpointer-arith
#AM_CFLAGS+=-Wcast-align
#AM_CFLAGS+=-Wcast-qual
#AM_CFLAGS+=-Wreturn-type
#AM_CFLAGS+=-Wextra
#AM_CFLAGS+=-Wstrict-prototypes
##AM_CFLAGS+=-Wmissing-declarations
##AM_CFLAGS+=-Wmissing-prototypes

AM_CPPFLAGS=
AM_CPPFLAGS+=-I$(top_srcdir)/libpbutil

#AM_CFLAGS+=-g
#AM_CFLAGS+=-ggdb
#AM_LDFLAGS+=-static-libgcc
#AM_LDFLAGS+=-static-libstdc++

#$(libffsox_2_a_OBJECTS): $(CONFIG_HEADER)
#$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil.h
#$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil_priv.h

noinst_LIBRARIES=lib1770_2.a
lib1770_2_a_SOURCES=$(CONFIG_HEADER)
lib1770_2_a_SOURCES+=$(top_srcdir)/libpbutil/pbutil.h
lib1770_2_a_SOURCES+=$(top_srcdir)/libpbutil/pbutil_priv.h
lib1770_2_a_SOURCES+=lib1770.h
lib1770_2_a_SOURCES+=lib1770_biquad.c
lib1770_2_a_SOURCES+=lib1770_block.c
lib1770_2_a_SOURCES+=lib1770_pre.c
lib1770_2_a_SOURCES+=lib1770_stats.c
