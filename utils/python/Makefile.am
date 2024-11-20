AM_CPPFLAGS = -I$(top_srcdir)/src       \
              -I$(top_srcdir)/utils		\
              -I$(top_builddir) # for config.h

BUILT_SOURCES = libctlgeom_wrap.c libctlgeom.py
EXTRA_DIST = $(BUILT_SOURCES) libctlgeom.i numpy.i
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

_libctlgeom_la_SOURCES = libctlgeom_wrap.c
_libctlgeom_la_LIBADD = $(top_builddir)/utils/libctlgeom.la $(PYTHON_LIBS)
_libctlgeom_la_LDFLAGS = -module -version-info @SHARED_VERSION_INFO@
_libctlgeom_la_CPPFLAGS = $(PYTHON_INCLUDES) $(AM_CPPFLAGS)

pyctlgeomdir = $(pythondir)/ctlgeom
pyctlgeom_PYTHON = __init__.py ctlgeom.py libctlgeom.py
pyctlgeom_LTLIBRARIES = _libctlgeom.la

#################################################
# generated code via swig and guile
#################################################

if MAINTAINER_MODE

LIBCTL_SWIG_SRC = libctlgeom.i numpy.i

HPPFILES=                                    \
 $(top_srcdir)/utils/ctlgeom.h               \
 $(top_srcdir)/src/ctl.h                     \
 $(top_srcdir)/utils/ctlgeom-types.h

libctlgeom_wrap.c: $(LIBCTL_SWIG_SRC) $(HPPFILES)
	$(SWIG) -Wextra $(SWIG_MEEP_FLAGS) $(AM_CPPFLAGS) -outdir $(builddir) -python -o $@ $(srcdir)/libctlgeom.i

libctlgeom.py: libctlgeom_wrap.c

endif # MAINTAINER_MODE
