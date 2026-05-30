if WITH_PYTHON
SUBDIRS = . python
else
SUBDIRS = .
endif

bin_SCRIPTS = gen-ctl-io
# ctlgeom-types.h and geom-ctl-io.c are checked in and hand-maintained —
# they carry C-only mesh internals (mesh_bvh_node, face_normals, the
# matching deep-copy code in mesh_copy/mesh_destroy, etc.) that don't
# have Scheme analogs in geom.scm. Treating them as regular sources
# (not BUILT_SOURCES) keeps Guile-detected builds from regenerating
# and clobbering those internals. See PR #74.
include_HEADERS = ctlgeom.h ctlgeom-types.h
lib_LTLIBRARIES = libctlgeom.la
noinst_PROGRAMS = geomtst

EXTRA_DIST = gen-ctl-io.in README geom.scm geom-ctl-io-defaults.c nlopt.c ctlgeom-types.h geom-ctl-io.c

libctlgeom_la_SOURCES = geom.c $(top_srcdir)/src/ctl-math.c $(top_srcdir)/src/integrator.c geom-ctl-io.c ctlgeom-types.h
libctlgeom_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
libctlgeom_la_CPPFLAGS = -DLIBCTLGEOM -I$(top_srcdir)/src

geomtst_SOURCES = geomtst.c
geomtst_LDADD = libctlgeom.la
geomtst_CPPFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = test-prism test-mesh

test_prism_SOURCES = test-prism.c
test_prism_LDADD   = libctlgeom.la
test_prism_CPPFLAGS = -I$(top_srcdir)/src

test_mesh_SOURCES = test-mesh.c
test_mesh_LDADD   = libctlgeom.la
test_mesh_CPPFLAGS = -I$(top_srcdir)/src

TESTS = test-prism test-mesh

dist_man_MANS = gen-ctl-io.1

BUILT_SOURCES = gen-ctl-io nlopt-constants.scm

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

endif

clean-local:
	rm -f ctl-io.[ch] nlopt-constants.scm geom-ctl-io.c
