bin_SCRIPTS = gen-ctl-io
include_HEADERS = ctlgeom.h
nodist_include_HEADERS = ctlgeom-types.h
lib_LTLIBRARIES = libctlgeom.la
noinst_PROGRAMS = geomtst

EXTRA_DIST = gen-ctl-io.in README geom.scm geom-ctl-io-defaults.c nlopt.c

libctlgeom_la_SOURCES = geom.c $(top_srcdir)/src/ctl-math.c $(top_srcdir)/src/integrator.c geom-ctl-io.c ctlgeom-types.h
libctlgeom_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
libctlgeom_la_CPPFLAGS = -DLIBCTLGEOM -I$(top_srcdir)/src

geomtst_SOURCES = geomtst.c
geomtst_LDADD = libctlgeom.la
geomtst_CPPFLAGS = -I$(top_srcdir)/src

dist_man_MANS = gen-ctl-io.1

BUILT_SOURCES = gen-ctl-io geom-ctl-io.c ctlgeom-types.h nlopt-constants.scm

nlopt-constants.scm:
	echo "#include <nlopt.h>" > nlopt-constants.h
	echo "; AUTOMATICALLY GENERATED - DO NOT EDIT" > $@
	names=`$(CPP) nlopt-constants.h 2>/dev/null | $(EGREP) 'NLOPT_[LG][ND]' | sed 's/ //g;s/_/-/g' |tr = , |cut -d, -f1`; i=0; for n in $$names; do echo "(define $$n $$i)" >> $@; i=`expr $$i + 1`; done
	rm nlopt-constants.h

if WITH_GUILE

ctl-io.c: geom.scm $(GEN_CTL_IO)
	$(GEN_CTL_IO) --c-only --code -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctl-io.h: geom.scm $(GEN_CTL_IO)
	$(GEN_CTL_IO) --c-only --header -o $@ $(srcdir)/geom.scm $(top_srcdir)

ctlgeom-types.h: ctl-io.h
	sed 's,SCM,void*,;s,ctl\.h,ctl-math.h,' ctl-io.h > $@

geom-ctl-io.c: ctl-io.c
	sed 's,ctl-io\.h,ctlgeom-types.h,;s,/.* Input variables .*/,@#include "geom-ctl-io-defaults.c"@#if 0@,;s,/.* Output variables .*/,#endif@,' ctl-io.c | tr '@' '\n' > $@

endif

clean-local:
	rm -f ctl-io.[ch] nlopt-constants.scm ctlgeom-types.h geom-ctl-io.c
