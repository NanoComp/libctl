lib_LTLIBRARIES = libctl.la
nodist_include_HEADERS = ctl.h

BUILT_SOURCES = ctl.h
EXTRA_DIST = ctl.h.in

libctl_la_SOURCES = ctl.c subplex.c ctl-f77-glue.c integrator.c cintegrator.c
libctl_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@
