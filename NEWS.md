# Libctl Release Notes

## libctl 4.5.1

2/1/22

* Ignore duplicate consecutive prism vertices (#60).

* Memory bug fixes (#58, #59).

## libctl 4.5.0

2/19/20

* New `make_slanted_prism` functions to make a prism with
  a given sidewall angle (#53).

* Defined `LIBCTL_MAJOR_VERSION` etc. in `ctlgeom.h` header file when
  using stand-alone libctlgeom.

* Bugfix in point-in-prism test (#49).

## libctl 4.4.0

11/12/19

* `geom_object_volume` function to get the volume of a 3d object
  (accelerates `box_overlap_with_object` for objects completely within a box) (#45).

* Bugfix to geometry tree search for empty dimensions.

## libctl 4.3.0

4/17/19

* `ctl_printf_callback` so that callers can capture stdout (#39).

## libctl 4.2.0

1/7/19

* Better handling of `center` parameter of prisms, allowing this
  to be optionally specified (#35).  Deprecates old `geom_fix_object`
  and `geom_fix_objects0` functions in favor of `geom_fix_object_ptr`
  and `geom_fix_object_list`.  (In particular, the old `geom_fix_object` routine will not work properly for prisms in
  which the center was not computed.)

## libctl 4.1.4

11/16/18

 * Work around gcc bug (closes #32).

 * Allow subclass properties to override defaults (or lack thereof) in parent class.

## libctl 4.1.3

9/7/2018

* Improved prism handling of points on prism faces (#23) and various cosmetic improvements (#22, #24, #25).

## libctl 4.1.2

7/27/2018

* Bug fix in prism subpixel averaging (#19 and #20), thanks to
  @DerekK88 for the bug report.

## libctl 4.1.1

6/29/2018

* Bug fix in prism bounding boxes (#17).

* Build fix for systems with an old `ctlgeom.h` file installed (#16).

## libctl 4.1

6/7/2018

* New "prism" geometric-object type for polygonal prisms (#13).

## libctl 4.0.1

4/18/2018

* Changed header file to use `const char *` rather than `char *`
  for constant string arguments, since C++ deprecated passing
  literal strings to `char *` arguments.

* Various minor build and compilation improvements.

## libctl 4.0

1/18/2018

* Building `--without-guile` is now possible to build only `libctlgeom`,
  which no longer depends on Guile.

* In libctlgeom, material is no represented by a `void*` rather than
  by a `struct` wrapping a `void*`.

* Migrate docs to github/markdown/readthedocs.

## libctl 3.2.2

3/28/2014.

  * Bug fix to interpolate-uniform for guile 1.8+.

## libctl 3.2.1

8/8/2012.

  * Fix incorrect gh_symbol2newstr macro replacement.

## libctl 3.2

7/20/2012.

  * Now works with Guile version 2.x (older versions are still supported).

  * Add `libctl_quiet` variable to main.c so that libctl-using programs
    can suppress all output if desired (e.g. to avoid duplicate outputs
    on parallel machines).

  * Added `wedge` object type for circular/cylindrical wedges, as a subclass
    of cylinder: `(make wedge (center ...) (axis ...) (radius ...) ...)` with
    two new properties: `(wedge-angle ...)` for the angle in radians, and
    `(wedge-start v)` for a vector v such that the wedge angles start at
    zero in the (v, axis) plane.  (Caveat: subpixel averaging is
    currently inaccurate for the flat wedge edges.)

  * list-type constructors now accept either `(name ...elements...)` or
    `(name (list ...elements...))`.

  * Add `vector3->exact` function for to-integer rounding.  Otherwise, ensure
    that interpolation results are floating-point to prevent type-conversion
    errors.

  * Added `ctl-set-prompt!` to set interactive prompt in both old and new
    Guile versions.

  * Rename `string` to `char*` in ctl-io.h for C++ compatibility.

  * Bug fix in `normal-to-object` near corners of blocks.

## libctl 3.1

6/5/2009.

  * Support specifying the location of the guile and guile-config
    programs with GUILE and GUILE_CONFIG environment variables in
    the configure script.

  * Support for calling NLopt optimization library (also requires
    the program using libctl to be changed to link nlopt).

  * New ellipsoid_overlap_with_object function, analogous to
    box_overlap_with_object function.

  * Bug fix in `include` function for recent versions of Guile,
    to properly keep track of the current include directory.

  * Bug fix in `numerical-derivative` routine, which didn't converge
    when the error was exactly zero.

## libctl 3.0.3

2/27/2008.

  * Added `begin-timed` function, which is similar to `begin-time` except
    that it returns the value of the last statement (like `begin`) rather
    than the time.

  * Bug fix: allow classes to have boolean properties.

  * Bug fixes for compilation under C++, thanks to David Foster:
    include missing string.h header and fixed gh_new_procedure prototype.

## libctl 3.0.2

8/22/2006.

  * Fix minor Guile incompatibility on some systems.

## libctl 3.0.1

5/1/2006.

  * Change shared-library version to 3:0:0 instead of 0:0:0.  This
    avoids conflicts with shared library version numbers that has
    been assigned to earlier versions of libctl for Debian; thanks to
    Josselin Mouette for the suggestion.

## libctl 3.0

4/1/2006.

  * Switch to use automake and libtool.  Can now install shared libraries
    with `--enable-shared`.

  * License is now GNU GPL (v2 or later) rather than the GNU LGPL, due
    to use of third-party GPL code for multi-dimensional integration (below).

  * `gen-ctl-io` now supports separate generation of code and header files
    via `--code` and `--header` arguments.  (Better for parallel make.)
    Also support a -o option to give a different output file name.

  * gen-ctl-io can now export C++ code by using the `--cxx` flag.

  * `gen-ctl-io` can now export SWIG `.i` files for automatic type conversion
    in SWIG wrapper generation, using the --swig flag.

  * Backwards incompatible change: users must include their own ctl-io.h
    *before* ctlgeom.h, or you get ctlgeom-types.h instead (this is
    for use with the `stand-alone` libctlgeom.a library below.

  * New multi-dimensional integration routines using adaptive cubature.
    (Much more efficient than nested 1d integrations.)  Adapted in part
    from the HIntlib Library by Rudolf Schuerer and from the GNU Scientific
    Library (GSL) by Brian Gough.

  * New `interpolate-uniform` function that tries to maintain a uniform
    distance between points (i.e. variable number of interpolated points
    between different list elements, as needed).

  * Now install a `stand-alone` libctlgeom.a library to make it easier
    to call geometry routines from non-Scheme code.

  * New routines to compute overlap fraction of box with object,
    compute analytical normal vectors, etcetera.  (For upcoming
    version of MPB.)  Also new routines to get the object of
    a point, not just the material.  Also new routines to operate
    on a supplied geometry list parameter instead of using the global;
    unlike the old `material_of_point_in_tree` functions, these
    functions do not shift the argument to the unit cell, but you
    can use the new function shift_to_unit_cell to get this behavior.

  * `gen-ctl-io` now generates object equal/copy functions.

  * In `unit-vector3`, only return 0 when norm==0, not merely if it is small.

  * Added one-sided numerical derivative routine.

  * Define `verbose?` variable corresponding to main.c variable.

  * `(print)` calls `(flush-all-ports)` to keep C and Scheme I/O in sync.

  * Fix in `find-root-deriv` to prevent infinite loop in some cases where
    the root does not exist; thanks to XiuLun Yang for the bug report.

  * Bug fix in `make_hermitian_cmatrix3x3`; thanks to Mischa Megens.

## libctl 2.2

9/12/2002.

  * Added simple trapezoidal-rule adaptive numeric integration routine.

  * Numerical derivative routines now allow numerical differentation
    of vector-valued function.  Added deriv2 convenience routine.

  * Added `find-root-deriv` functions for faster root-finding of
    functions for which the derivative is also available.

  * Added missing `(cvector3 ...)` constructor, and fixed corresponding
    constructor for `cvector3` object properties; thanks to Doug Allan for
    the bug report.

  * Added generic `memoize` function.

  * libctl programs now print out command-line parameters when they run.

  * Fixed incomplete support for generic SCM type.

  * Fixed to work with Guile 1.5+ (thanks to Mike Watts for the bug report).

## libctl 2.1

3/21/2002.

  * Bug fix: complex-number input variables were read as garbage
    if they had imaginary parts; does not affect complex-number outputs.

  * Added generic SCM type for i/o variables and parameters, as a
    catch-all for other Scheme objects.

  * main.c now has `ctl_export_hook` (enabled by defining
    CTL_HAVE_EXPORT_HOOK) with which to define additional Guile symbols.

  * gen-ctl-io: converts `!` in symbols to `B` in C identifiers.

## libctl 2.0

3/10/2002.

  * New `set-param!` function, analogous to define-param, that allows
    you to change the value of a parameter in a way that can still be
    overridden from the command line.

  * In libgeom, allow user to specify the `resolution` instead of the
    `grid-size`.  New `no-size` support in lattice class to reduce
    dimensionality, and new `(get-grid-size)` function.

  * Support for Scheme complex numbers, along with a few new associated
    functions: `conj`, `vector3-cdot`, `matrix3x3-adjoint`.

  * New functions to compute numerical derivatives using Ridder's
    method of polynomial extrapolation.

  * Documented `object-property-value`; thanks to Theis Peter Hansen for
    the suggestion.

  * Get rid of unneeded `make-default`, and use consistent syntax for
    `define-property` and `define-post-processed-property`, compared to
    `define-input-var`.  NOT BACKWARD COMPATIBLE (for developers; users
    are not affected).  Thanks to Theis Peter Hansen for the suggestion.

  * Call ctl_stop_hook even with `--help`, `--version`, etcetera; this
    makes the behavior nicer e.g. with MPI.

## libctl 1.5

11/15/2001.

  * geometry-lattice now has a separate basis-size property, so that you
    can specify the basis vectors as being something other than unit vectors.

  * More functions are tail-recursive, helping to prevent stack overflows;
    thanks to Robert Sheldon for the bug report.

  * New fold-left and fold-right functions, documented in the manual.

  * The configure script now checks that guile is in the $PATH.  Thanks to
    Bing Li and Giridhar Malalahalli for their bug reports.

## libctl 1.4.1

7/4/2001.

  * Support function lists.

## libctl 1.4

2/23/2001.

  * Renamed `display-many` function to more felicitous `print`
    and added `print-ok?` global variable that allows you
    to disable program output.

  * Added support for passing `'function` types back and forth
    (just a SCM object pointing to a Scheme function).

  * Cosmetic fixes to yes/no? and menu-choice interaction functions.

  * Support start/exit hooks for use e.g. with MPI.

## libctl 1.3

1/7/2001.

  * Added improved `subplex` multidimensional optimization algorithm
    (for maximize-multiple and minimize-multiple).

  * Documented vector3-x, vector3-y, vector3-z functions for extracting
    vector3 components.

## libctl 1.2

7/9/2000.

  * Added new `cone` geometric object type.

  * Added reciprocal->cartesian, cartesian->lattice, lattice->reciprocal,
    etcetera functions to libgeom for converting vectors between bases.

  * Added routines rotate-vector3 and rotation-matrix3x3 for rotating vectors.

  * Added support for returning lists from external functions.

  * Fixed bug in matrix3x3-inverse function.

  * Fixed bug in find-root for converging to negative roots.

  * Use Nelder-Mead simplex algorithm for multi-dimensional minimization
    (it seems to be more robust than the old routine).

## libctl 1.1.1

1/28/2000.

  * Use CPPFLAGS environment variable instead of the less-standard
    INCLUDES to pass -I flags to the configure script (for header files
    in non-standard locations).

  * Compilation fixes.  We need to set SHELL in the Makefile for make on
    some systems.  Also added rule to insure ctl-io.h is created before
    main.c is compiled.  Thanks to Christoph Becher for the bug reports.

## libctl 1.1

1/2/2000.

  * geom: radius and height of objects is now permitted to be zero.

  * geom: `material_of_point_*` routines now report whether the point
    is in any object; necessary for use with MPB 0.9.

  * Added man page for `gen-ctl-io`, based on a contribution by
    James R. Van Zandt.

## libctl 1.0.1

11/22/1999.

  * geom: handle case where `ensure-periodicity` is `false`.

  * geom: bug fix in `geometric-objects-lattice-duplicates` for
    non-orthogonal lattices; thanks to Karl Koch for the bug report.

## libctl 1.0

11/19/1999.

  * Initial public release.
