# Process this file with autoconf to produce a configure script.
AC_INIT(libctl, 4.0.1, stevenj@alum.mit.edu)
AC_CONFIG_SRCDIR([src/ctl.c])
AC_CONFIG_HEADERS([config.h src/ctl.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Shared-library version number; indicates api compatibility, and is
# not the same as the "public" version number.  (Don't worry about this
# except for public releases.)
SHARED_VERSION_INFO="6:0:0" # CURRENT:REVISION:AGE

AM_INIT_AUTOMAKE([foreign])
AC_SUBST(SHARED_VERSION_INFO)
AM_ENABLE_SHARED(no) dnl shared libs cause too many headaches to be default
AC_PROG_LIBTOOL

###########################################################################

LIBCTL_VERSION=$PACKAGE_VERSION
AC_DEFINE_UNQUOTED(LIBCTL_VERSION, "$LIBCTL_VERSION",
	   	   [Define to version string for libctl.])
AC_SUBST(LIBCTL_VERSION)

LIBCTL_MAJOR_VERSION=`echo $LIBCTL_VERSION |cut -d. -f1`
LIBCTL_MINOR_VERSION=`echo $LIBCTL_VERSION |cut -d. -f2`
LIBCTL_BUGFIX_VERSION=`echo $LIBCTL_VERSION |cut -d. -f3`
test "x$LIBCTL_BUGFIX_VERSION" = x && LIBCTL_BUGFIX_VERSION=0
AC_DEFINE_UNQUOTED(LIBCTL_MAJOR_VERSION, $LIBCTL_MAJOR_VERSION, [major v.])
AC_DEFINE_UNQUOTED(LIBCTL_MINOR_VERSION, $LIBCTL_MINOR_VERSION, [minor v.])
AC_DEFINE_UNQUOTED(LIBCTL_BUGFIX_VERSION, $LIBCTL_BUGFIX_VERSION, [bugfix v.])

###########################################################################

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

AC_CHECK_PROGS(INDENT, indent gindent, echo)
AC_SUBST(INDENT)

# check how to transform the name of the installed program:

AC_ARG_ENABLE(debug, [  --enable-debug          compile for debugging], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
        CFLAGS="-g"
        AC_DEFINE(DEBUG, 1, [Define to enable debugging checks.])
fi

# Checks for header files.
AC_HEADER_STDC

###########################################################################

# Find Guile library, flags, etcetera:

AC_ARG_WITH(guile, [AC_HELP_STRING([--without-guile],[disable use of Guile])],
                   with_guile=$withval, with_guile=yes)

if test x"$with_guile" = xyes; then
	AC_CHECK_PROG(GUILE, guile, guile, unknown)
	if test x"$guile_ok" = xunknown; then
		AC_MSG_WARN([could not find guile program; check your PATH ... disabling guile])
		with_guile=no
	fi
fi
AC_SUBST(GUILE)

# seems to be needed on ubuntu (issue #1):
AC_CHECK_LIB(m, sqrt)

noguile_LIBS=$LIBS
LIBGUILE=""

if test x"$with_guile" = xyes; then
	AC_CHECK_PROG(GUILE_CONFIG, guile-config, guile-config, unknown)
	if test "x$GUILE_CONFIG" = "xunknown"; then
		AC_CHECK_LIB(readline, readline)
		AC_CHECK_LIB(dl, dlopen)
		ok=no
		AC_CHECK_LIB(guile, gh_eval_str, [ok=yes],
		             [AC_CHECK_LIB(guile, scm_eval_string, [ok=yes])])
        if test $ok = yes; then
           LIBGUILE="-lguile -ldl -lreadline"
		   LIBS="-lguile $LIBS"
		   AC_DEFINE(HAVE_LIBGUILE, 1, [Define if we have -lguile])
	    else
		   with_guile=no
		fi
	else
		CPPFLAGS="$CPPFLAGS `$GUILE_CONFIG compile`"
        LIBGUILE=`$GUILE_CONFIG link`
		LIBS="$LIBS $LIBGUILE"
	fi
fi


if test x"$with_guile" = xyes; then
	AC_MSG_CHECKING([if linking to guile works])
	AC_TRY_LINK_FUNC(gh_enter, AC_MSG_RESULT(yes),
	   [AC_TRY_LINK_FUNC(scm_boot_guile, AC_MSG_RESULT(yes),
	       [AC_MSG_RESULT(no)
	        AC_MSG_ERROR(Guile linking failed)])])
	AC_CHECK_HEADERS([libguile.h guile/gh.h])
fi

AC_SUBST(LIBGUILE)
AM_CONDITIONAL(WITH_GUILE, test "x$with_guile" != xno)

echo "######################### $with_guile ##############################"

###########################################################################

# Checks for Guile features:

if test x"$with_guile" = xyes; then

AC_MSG_CHECKING([for modern non-gh interface])
ok=yes
AC_TRY_LINK([#include <libguile.h>
], [scm_from_double(0.0);scm_from_int(0);scm_boot_guile(0,0,0,0);scm_c_define_gsubr(0,0,0,0,0);], [AC_DEFINE(HAVE_NO_GH, 1, [Define if we can avoid the gh interface])], ok=no)
AC_MSG_RESULT($ok)


AC_MSG_CHECKING([for gh_enter])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_enter(0,0,0);], [AC_DEFINE(HAVE_GH_ENTER, 1, [Define if we have gh_enter.])], ok=no)
AC_MSG_RESULT($ok)

AC_MSG_CHECKING([for gh_eval_str])
ok=yes
AC_TRY_LINK([#include <guile/gh.h>
], [gh_eval_str(0);], [AC_DEFINE(HAVE_GH_EVAL_STR, 1, [Define if we have gh_eval_str.])], ok=no)
AC_MSG_RESULT($ok)

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
AC_TRY_LINK([#if defined(HAVE_LIBGUILE_H)
#  include <libguile.h>
#else
#  include <guile/gh.h>
#endif
], [scm_flush_all_ports();], [AC_DEFINE(HAVE_SCM_FLUSH_ALL_PORTS, 1, [Define if we have scm_flush_all_ports])], ok=no)
AC_MSG_RESULT($ok)

AC_CHECK_FUNCS(scm_make_complex scm_c_make_rectangular scm_variable_set_x scm_c_lookup scm_c_make_vector scm_variable_ref)

AC_MSG_CHECKING([for SCM_COMPLEXP])
ok=yes
AC_TRY_LINK([#if defined(HAVE_LIBGUILE_H)
#  include <libguile.h>
#else
#  include <guile/gh.h>
#endif
], [SCM x; SCM_COMPLEXP(x);], [AC_DEFINE(HAVE_SCM_COMPLEXP, 1, [Define if we have SCM_COMPLEXP])], ok=no)
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

fi # with_guile

LIBS=$noguile_LIBS

###########################################################################

# Find Fortran name-mangling routines, for wrapper functions

AC_PROG_F77
if test -z "$F77"; then
   AC_MSG_WARN([Fortran wrapper functions will not be included])
else
   AC_F77_WRAPPERS
fi

###########################################################################
# Check for C99 complex-number support, for cintegrate.c

AC_CHECK_HEADERS(complex.h)
AC_CACHE_CHECK([for C complex keyword], acx_cv_c_complex,
 [acx_cv_c_complex=unsupported
  for acx_kw in complex _Complex __complex__; do
    AC_TRY_COMPILE([#include <complex.h>], [float $acx_kw foo;],
                   [acx_cv_c_complex=$acx_kw; break])
  done
 ])

if test "$acx_cv_c_complex" = "unsupported"; then
   AC_MSG_WARN([C doesn't support complex numbers; disabling complex integration.])
else
   AC_DEFINE([CTL_HAS_COMPLEX_INTEGRATION], [1], [If we have C99 complex nums])
fi

###########################################################################
# Check for nlopt, or at least its header, and extract Scheme constants

AC_PROG_EGREP
AC_CHECK_HEADERS(nlopt.h)

###########################################################################

libctl_dir=$srcdir
case $libctl_dir in .*) libctl_dir=`pwd`/$libctl_dir ;; esac

LIBCTL_DIR="$libctl_dir"
AC_SUBST(LIBCTL_DIR)

GEN_CTL_IO="`pwd`/utils/gen-ctl-io"
AC_SUBST(GEN_CTL_IO)

# On IRIX, basename/dirname functions require -lgen
AC_CHECK_LIB(gen, basename)

###########################################################################

AC_CONFIG_FILES([Makefile src/Makefile utils/Makefile examples/Makefile examples/example.scm])
AC_CONFIG_FILES([utils/gen-ctl-io], [chmod u+x utils/gen-ctl-io])
AC_OUTPUT
