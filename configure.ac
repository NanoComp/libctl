# Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([base/ctl.c])
AC_CONFIG_HEADER(base/ctl.h)

LIBCTL_VERSION=2.1beta
AC_DEFINE_UNQUOTED(LIBCTL_VERSION, "$LIBCTL_VERSION",
	   	   [Define to version string for libctl.])
AC_SUBST(LIBCTL_VERSION)

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_CHECK_PROGS(INDENT, indent gindent, echo)
AC_SUBST(INDENT)

# check how to transform the name of the installed program:
AC_ARG_PROGRAM

AC_ARG_ENABLE(debug, [  --enable-debug          compile for debugging], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
        CFLAGS="-g"
        AC_DEFINE(DEBUG, 1, [Define to enable debugging checks.])
fi

AC_CANONICAL_HOST

# Checks for header files.
AC_HEADER_STDC

###########################################################################

# Find Guile library, flags, etcetera:

AC_CHECK_PROG(guile_ok, guile, yes, no)
if test x"$guile_ok" = xno; then
	AC_MSG_ERROR([could not find guile program; check your PATH])
fi

AC_CHECK_PROG(HAVE_GUILE_CONFIG, guile-config, yes, no)
if test "$HAVE_GUILE_CONFIG" = "yes"; then
	CPPFLAGS="$CPPFLAGS `guile-config compile`"
	LIBS="$LIBS `guile-config link`"
else
	AC_CHECK_LIB(m, sqrt)
	AC_CHECK_LIB(readline, readline)
	AC_CHECK_LIB(dl, dlopen)
	AC_CHECK_LIB(guile, gh_eval_str)
fi

AC_MSG_CHECKING([if linking to guile works])
AC_TRY_LINK_FUNC(gh_enter, AC_MSG_RESULT(yes), [
AC_MSG_RESULT(no)
AC_MSG_ERROR(Guile could not be found)
])

###########################################################################

# Checks for Guile features:

AC_MSG_CHECKING([for gh_load])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_load(0);], [AC_DEFINE(HAVE_GH_LOAD, 1, [Define if we have gh_load.])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for gh_bool2scm])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_bool2scm(0);], [AC_DEFINE(HAVE_GH_BOOL2SCM, 1, [Define if we have gh_bool2scm])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for gh_vector_ref])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_vector_ref(0,0);], [AC_DEFINE(HAVE_GH_VECTOR_REF, 1, [Define if we have gh_vector_ref])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for gh_list_ref])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_list_ref(0,0);], [AC_DEFINE(HAVE_GH_LIST_REF, 1, [Define if we have gh_list_ref])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for gh_length])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_length(0);], [AC_DEFINE(HAVE_GH_LENGTH, 1, [Define if we have gh_length])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for scm_flush_all_ports])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [scm_flush_all_ports();], [AC_DEFINE(HAVE_SCM_FLUSH_ALL_PORTS, 1, [Define if we have scm_flush_all_ports])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([whether gh_lookup works properly])
ok=yes
AC_TRY_RUN([
#include <stdlib.h>
#include <guile/gh.h>
void main_entry(int argc, char *argv[])
{
     gh_eval_str("(define foo 3.14159)");
     if (SCM_UNDEFINED == gh_lookup("foo"))
          exit(EXIT_FAILURE);
}
int main (int argc, char *argv[])
{
     gh_enter (argc, argv, main_entry);
     return EXIT_SUCCESS;
}
], [AC_DEFINE(GH_LOOKUP_OK, 1, [Define if gh_lookup works])], ok=no, ok=no)
AC_MSG_RESULT($ok)

###########################################################################

# Add libctl and libgeom -I/-L/-l flags to CPPFLAGS/LDFLAGS/LIBS,
# so that examples/Makefile.in can be used as-is for other programs.
# (The flags need to go first so that they'll take precedence over
# any previously-installed version of libctl.)

CPPFLAGS="-I`pwd`/$srcdir/base -I`pwd`/$srcdir/utils $CPPFLAGS"
LDFLAGS="-L`pwd`/$srcdir/base $LDFLAGS"

###########################################################################

LIBS="-lctl $LIBS"

LIBCTL_DIR="`pwd`/$srcdir"
AC_SUBST(LIBCTL_DIR)

GEN_CTL_IO="`pwd`/$srcdir/utils/gen-ctl-io"
AC_SUBST(GEN_CTL_IO)

AC_CONFIG_FILES([Makefile base/Makefile utils/Makefile examples/Makefile utils/gen-ctl-io])
AC_OUTPUT
