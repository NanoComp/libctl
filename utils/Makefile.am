bin_SCRIPTS = gen-ctl-io
include_HEADERS = ctlgeom.h
nodist_include_HEADERS = ctlgeom-types.h
lib_LTLIBRARIES = libctlgeom.la

EXTRA_DIST = gen-ctl-io.in README geom.scm

BUILT_SOURCES = gen-ctl-io ctl-io.c ctl-io.h ctlgeom-types.h

libctlgeom_la_SOURCES = geom.c 
nodist_libctlgeom_la_SOURCES = ctl-io.c ctl-io.h
libctlgeom_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@

dist_man_MANS = gen-ctl-io.1

ctl-io.c: geom.scm
	$(GEN_CTL_IO) --code -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctl-io.h: geom.scm
	$(GEN_CTL_IO) --header -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctlgeom-types.h: ctl-io.h
	cp -f ctl-io.h $@

clean-local:
	rm -f ctl-io.[ch] ctl-io.*o ctlgeom-types.h
