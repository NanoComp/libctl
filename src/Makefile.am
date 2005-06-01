lib_LTLIBRARIES = libctl.la
include_HEADERS = ctl.h

BUILT_SOURCES = ctl.h
EXTRA_DIST = ctl.h.in

libctl_la_SOURCES = ctl.c subplex.c ctl-f77-glue.c
libctl_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
