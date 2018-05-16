
EXTRA_DIST = ctl.h.in

if WITH_GUILE
lib_LTLIBRARIES = libctl.la
endif

include_HEADERS = ctl-math.h
nodist_include_HEADERS = ctl.h

BUILT_SOURCES = ctl.h

libctl_la_SOURCES = ctl.c ctl-math.c subplex.c ctl-f77-glue.c integrator.c cintegrator.c
libctl_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@
libctl_la_LIBADD = $(LIBGUILE)
