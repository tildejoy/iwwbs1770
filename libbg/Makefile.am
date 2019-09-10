AM_CPPFLAGS=
AM_CPPFLAGS+=-I$(top_srcdir)/libpbutil
AM_CPPFLAGS+=-I$(top_srcdir)/libff
AM_CPPFLAGS+=-I$(top_srcdir)/lib1770-2

bin_PROGRAMS=bs1770gain
bs1770gain_SOURCES=bgx.c
bs1770gain_LDADD=
bs1770gain_LDADD+=libbg.a
bs1770gain_LDADD+=$(top_builddir)/libff/libff.a
bs1770gain_LDADD+=$(top_builddir)/lib1770-2/lib1770_2.a
bs1770gain_LDADD+=$(top_builddir)/libpbutil/libpbutil.a
nodist_bs1770gain_SOURCES=bg_version.h

noinst_PROGRAMS=

#noinst_PROGRAMS+=pilot
#pilot_SOURCES=
#pilot_SOURCES+=bg.h
#pilot_SOURCES+=bg_pilot.h
#pilot_SOURCES+=$(top_srcdir)/libff/ff.h
#pilot_SOURCES+=pilot.c
#pilot_LDADD=
#pilot_LDADD+=libbg.a
#pilot_LDADD+=$(top_builddir)/libff/libff.a
#pilot_LDADD+=$(top_builddir)/lib1770-2/lib1770_2.a
#pilot_LDADD+=$(top_builddir)/libpbutil/libpbutil.a

noinst_PROGRAMS+=bg_version
bg_version_SOURCES=bg_version.c

noinst_LIBRARIES=libbg.a
libbg_a_SOURCES=
libbg_a_SOURCES+=$(CONFIG_HEADER)
libbg_a_SOURCES+=bg.h
libbg_a_SOURCES+=bg_pilot.h
libbg_a_SOURCES+=$(top_srcdir)/libff/ff.h
libbg_a_SOURCES+=$(top_srcdir)/lib1770-2/lib1770.h
libbg_a_SOURCES+=$(top_srcdir)/libpbutil/pbutil.h
libbg_a_SOURCES+=$(top_srcdir)/libpbutil/pbutil_priv.h
libbg_a_SOURCES+=bg_param.c
libbg_a_SOURCES+=bg_print_conf.c
libbg_a_SOURCES+=bg_print_xml.c
libbg_a_SOURCES+=bg_print_classic.c
libbg_a_SOURCES+=bg_annotator.c
libbg_a_SOURCES+=bg_muxer.c
libbg_a_SOURCES+=bg_analyzer.c
libbg_a_SOURCES+=bg_tree.c
libbg_a_SOURCES+=bg_tree_path.c
libbg_a_SOURCES+=bg_file.c
libbg_a_SOURCES+=bg_track.c
libbg_a_SOURCES+=bg_track_target.c
libbg_a_SOURCES+=bg_tree_patha.c
libbg_a_SOURCES+=bg_album.c
libbg_a_SOURCES+=bg_root.c
libbg_a_SOURCES+=bg_pilot_branch.c
libbg_a_SOURCES+=bg_pilot_leaf.c
libbg_a_SOURCES+=bg_pilot.c
libbg_a_SOURCES+=bg_basenamea.c
libbg_a_SOURCES+=bg_basename.c
libbg_a_SOURCES+=bg_wcs2str.c
libbg_a_SOURCES+=bg_pathnorm.c
libbg_a_SOURCES+=bg_parse_time.c

# https://www.gnu.org/software/automake/manual/html_node/Built-Sources-Example.html
BUILT_SOURCES=bg_version.h
CLEANFILES=bg_version.h
bg_version.h: bg_version
	./$< > ./$@
