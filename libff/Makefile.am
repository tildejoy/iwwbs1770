AM_CPPFLAGS=
AM_CPPFLAGS+=-I$(top_srcdir)/libpbutil
AM_CPPFLAGS+=-I$(top_srcdir)/lib1770-2

$(libff_a_OBJECTS): $(CONFIG_HEADER)
$(libff_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil.h
$(libff_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil_priv.h
$(libff_a_OBJECTS): $(top_srcdir)/lib1770-2/lib1770.h

noinst_LIBRARIES=libff.a
libff_a_SOURCES=
libff_a_SOURCES+=ff.h
libff_a_SOURCES+=ff_muxer.c
libff_a_SOURCES+=ff_fifo.c
libff_a_SOURCES+=ff_filter.c
libff_a_SOURCES+=ff_analyzer.c
libff_a_SOURCES+=ff_resampler.c
libff_a_SOURCES+=ff_inout.c
libff_a_SOURCES+=ff_printer.c
libff_a_SOURCES+=ff_audio.c
libff_a_SOURCES+=ff_iter.c
libff_a_SOURCES+=ff_wcs2str.c
libff_a_SOURCES+=ff_csv2avdict.c
libff_a_SOURCES+=ff_mv.c
libff_a_SOURCES+=ff_fcmp.c
libff_a_SOURCES+=ff_fexists.c
libff_a_SOURCES+=ff_mkdir.c
libff_a_SOURCES+=ff_rm.c
libff_a_SOURCES+=ff_dynload.c
