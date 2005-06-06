bin_SCRIPTS = gen-ctl-io
include_HEADERS = ctlgeom.h
nodist_include_HEADERS = ctlgeom-types.h
lib_LTLIBRARIES = libctlgeom.la

EXTRA_DIST = gen-ctl-io.in README geom.scm geom-ctl-io-defaults.c

BUILT_SOURCES = gen-ctl-io geom-ctl-io.c ctl-io.c ctl-io.h ctlgeom-types.h

libctlgeom_la_SOURCES = geom.c 
nodist_libctlgeom_la_SOURCES = geom-ctl-io.c ctl-io.h
libctlgeom_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@

dist_man_MANS = gen-ctl-io.1

ctl-io.c: geom.scm
	$(GEN_CTL_IO) --code -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctl-io.h: geom.scm
	$(GEN_CTL_IO) --header -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctlgeom-types.h: ctl-io.h
	cp -f ctl-io.h $@

geom-ctl-io.c: ctl-io.c
	sed 's,/.* Input variables .*/,\0@#include "geom-ctl-io-defaults.c"@#if 0@,;s,/.* Output variables .*/,#endif@\0,' ctl-io.c | tr '@' '\n' > $@

clean-local:
	rm -f ctl-io.[ch] geom-ctl-io.c ctlgeom-types.h
