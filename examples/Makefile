SPECIFICATION_FILE = photonic-crystal.scm
PROGRAM_NAME = example
LIBCTL_DIR = ..

MY_OBJECTS = main.o
MY_HEADERS = 

##############################################################################

CC = gcc
CFLAGS = -O3 -Wall

##############################################################################

LIBCTL_SCM_SRC = $(LIBCTL_DIR)/src/ctl.scm
LIBCTL_C_SRC = $(LIBCTL_DIR)/src/ctl.c
LIBCTL_OBJ = $(LIBCTL_C_SRC:.c=.o)
LIBCTL_HEADERS = $(LIBCTL_C_SRC:.c=.h)

CTL_IO_GEN_SRC = $(LIBCTL_DIR)/utils/ctl-io.scm
CTL_IO_GEN = $(LIBCTL_DIR)/utils/gen-ctl-io

INCLUDE_FLAGS = -I$(LIBCTL_DIR)/src

OBJECTS = $(MY_OBJECTS) $(LIBCTL_OBJ) ctl-io.o
HEADERS = $(MY_HEADERS) $(LIBCTL_HEADERS)

##############################################################################

all: $(PROGRAM_NAME)

ctl-io.c: $(SPECIFICATION_FILE) \
          $(LIBCTL_SCM_SRC) $(CTL_IO_GEN_SRC) $(CTL_IO_GEN)
	$(CTL_IO_GEN) $(SPECIFICATION_FILE)

ctl-io.h: $(SPECIFICATION_FILE) \
          $(LIBCTL_SCM_SRC) $(CTL_IO_GEN_SRC) $(CTL_IO_GEN)
	$(CTL_IO_GEN) $(SPECIFICATION_FILE)

$(PROGRAM_NAME): $(OBJECTS) $(HEADERS)
	gcc $(OBJECTS) -lguile -lm -o $@

.c.o: $(HEADERS)
	$(CC) -c $(INCLUDE_FLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f $(MY_OBJECTS) ctl-io.o ctl-io.c ctl-io.h

realclean: clean
	rm -f $(OBJECTS)
