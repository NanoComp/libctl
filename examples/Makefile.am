# libctl: flexible Guile-based control files for scientific software
# Copyright (C) 1998, 1999, 2000, 2001, 2002, Steven G. Johnson
#
# This file may be used without restriction.  It is in the public
# domain, and is NOT restricted by the terms of any GNU license.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# Steven G. Johnson can be contacted at stevenj@alum.mit.edu.

##############################################################################

# Makefile.am for programs using libctl (assumes that automake is
# being used, of course).  You should only need to change the definitions
# in this section of the file.  Replace "example" with the name of
# your program.

# change "noinst" to "bin" to be able to install program with 'make install'
noinst_PROGRAMS = example

SPECIFICATION_FILE = example.scm
EXTRA_DIST = example.scm README run.ctl

MY_SOURCES = example.c # plus any other .c/.h files you need

MY_LIBS    =         # extra libs you need go here
MY_LDFLAGS =         # extra -L flags go here
MY_CPPFLAGS =        # extra -I include flags go here
MY_DEFS =            # extra -D define flags go here

# what is printed out when invoking your program with --version:
VERSION_STRING = "example @VERSION@"

# The following variables should be detected and set by autoconf:

# libctl install. dir., e.g. /usr/local/share/libctl
LIBCTL_DIR = @LIBCTL_DIR@

# gen-ctl-io program
GEN_CTL_IO = @GEN_CTL_IO@

##############################################################################
#   don't (normally) edit below except to replace "example" with your name   #
##############################################################################

CTL_DEFS = -DCTL_SCM='"'$(LIBCTL_DIR)/base/ctl.scm'"' \
       -DINCLUDE_SCM='"'$(LIBCTL_DIR)/base/include.scm'"' \
          -DSPEC_SCM='"'$(prefix)/share/libctl/specs/$(SPECIFICATION_FILE)'"' \
    -DVERSION_STRING='"'$(VERSION_STRING)'"'

example_SOURCES = $(MY_SOURCES)
nodist_example_SOURCES = main.c geom.c ctl-io.h ctl-io.c
BUILT_SOURCES = $(nodist_example_SOURCES)
example_LDADD = $(MY_LIBS) -lctl
example_LDFLAGS = $(MY_LDFLAGS)
example_CPPFLAGS = $(MY_CPPFLAGS) $(MY_DEFS) $(CTL_DEFS)

main.c: $(LIBCTL_DIR)/base/main.c
	cp -f $(LIBCTL_DIR)/base/main.c $@

geom.c: $(LIBCTL_DIR)/utils/geom.c
	cp -f $(LIBCTL_DIR)/utils/geom.c $@

ctl-io.c ctl-io.h: $(SPECIFICATION_FILE)
	$(GEN_CTL_IO) $(srcdir)/$(SPECIFICATION_FILE) $(LIBCTL_DIR)

clean-local:
	rm -f ctl-io.* main.* geom.*
