# libctl: flexible Guile-based control files for scientific software
# Copyright (C) 1998 Steven G. Johnson
#
# This file may be used without restriction.  It is in the public
# domain, and is NOT restricted by the terms of any GNU license.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# Steven G. Johnson can be contacted at stevenj@alum.mit.edu.

SPECIFICATION_FILE = photonic-crystal.scm
PROGRAM_NAME = example
LIBCTL_DIR = ..

MY_OBJECTS = main.o example.o
MY_HEADERS = 

##############################################################################

CC = gcc
CFLAGS = -g -Wall

##############################################################################

LIBCTL_SCM_SRC = $(LIBCTL_DIR)/src/ctl.scm
LIBCTL_C_SRC = $(LIBCTL_DIR)/src/ctl.c
LIBCTL_OBJ = $(LIBCTL_C_SRC:.c=.o)
LIBCTL_HEADERS = $(LIBCTL_C_SRC:.c=.h)

CTL_IO_GEN_SRC = $(LIBCTL_DIR)/utils/ctl-io.scm
CTL_IO_GEN = $(LIBCTL_DIR)/utils/gen-ctl-io

INCLUDE_FLAGS = -I$(LIBCTL_DIR)/src

DEFINE_FLAGS = -DCTL_SCM='"'$(PWD)/$(LIBCTL_SCM_SRC)'"' \
               -DSPEC_SCM='"'$(PWD)/$(SPECIFICATION_FILE)'"' \
	       -DHAVE_GUILE_1_3

OBJECTS = $(MY_OBJECTS) $(LIBCTL_OBJ) ctl-io.o
HEADERS = $(MY_HEADERS) $(LIBCTL_HEADERS) ctl-io.h

##############################################################################

all: $(PROGRAM_NAME)

ctl-io.c: $(SPECIFICATION_FILE) \
          $(LIBCTL_SCM_SRC) $(CTL_IO_GEN_SRC) $(CTL_IO_GEN)
	$(CTL_IO_GEN) $(SPECIFICATION_FILE)

ctl-io.h: $(SPECIFICATION_FILE) \
          $(LIBCTL_SCM_SRC) $(CTL_IO_GEN_SRC) $(CTL_IO_GEN)
	$(CTL_IO_GEN) $(SPECIFICATION_FILE)

$(PROGRAM_NAME): $(HEADERS) $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -lguile -lm -o $@

.c.o: $(HEADERS)
	$(CC) -c $(DEFINE_FLAGS) $(INCLUDE_FLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f $(MY_OBJECTS) ctl-io.o ctl-io.c ctl-io.h $(PROGRAM_NAME) core

realclean: clean
	rm -f $(LIBCTL_OBJ)
