---
# Libctl release notes
---

Here, we describe what has changed between releases of the [libctl](index.md) package. You can also refer to the `NEWS` file in the libctl package (or the `ChangeLog` file for a more detailed listing).

libctl 3.2.2
------------

<small>28 March 2014</small>

-   Bug fix to `interpolate-uniform` for guile 1.8+.

libctl 3.2.1
------------

<small>8 August 2012</small>

-   Fix incorrect `gh_symbol2newstr` macro replacement.

libctl 3.2
----------

<small>20 July 2012</small>

-   Now works with Guile version 2.x (older versions are still supported).
-   Add `libctl_quiet` variable to `main.c` so that libctl-using programs can suppress all output if desired (e.g. to avoid duplicate outputs on parallel machines).
-   Added `wedge` object type for circular/cylindrical wedges, as a subclass of `cylinder`: `(make` `wedge` `(center` `...)` `(axis` `...)` `(radius` `...)` `...)` with two new properties: `(wedge-angle` `...)` for the angle in radians, and `(wedge-start` `v)` for a vector v such that the wedge angles start at zero in the (v, axis) plane. $$Caveat: subpixel averaging is currently inaccurate for the flat wedge edges.$$
-   list-type constructors now accept either `(name` `...elements...)` or `(name` `(list` `...elements...))`.
-   Add `vector3->exact` function for to-integer rounding. Otherwise, ensure that interpolation results are floating-point to prevent type-conversion errors.
-   Added `ctl-set-prompt!` to set interactive prompt in both old and new Guile versions.
-   Rename `string` to `char*` in `ctl-io.h` for C++ compatibility.
-   Bug fix in normal-to-object near corners of blocks.

libctl 3.1
----------

<small>5 June 2008</small>

-   Support specifying the location of the `guile` and `guile-config` programs with `GUILE` and `GUILE_CONFIG` environment variables in the `configure` script.
-   Support for calling [NLopt](http://nlopt.readthedocs.io/en/latest/NLopt) optimization library (also requires the program using libctl to be changed to link nlopt).
-   New `ellipsoid_overlap_with_object` function, analogous to `box_overlap_with_object` function.
-   Bug fix in `include` function for recent versions of Guile, to properly keep track of the current include directory.
-   Bug fix in numerical-derivative routine, which didn't converge when the error was exactly zero.

libctl 3.0.3
------------

<small>27 February 2008</small>

-   Added "`begin-timed`" function, which is similar to "`begin-time`" except that it returns the value of the last statement (like "`begin`") rather than the time.
-   Bug fix: allow classes to have `boolean` properties.
-   Bug fixes for compilation under C++, thanks to David Foster: include missing `string.h` header and fixed `gh_new_procedure` prototype.

libctl 3.0.2
------------

<small>22 August 2006</small>

-   Fix minor Guile incompatibility on some systems.

libctl 3.0.1
------------

<small>5/1/2006</small>

-   Change shared-library version to 3:0:0 instead of 0:0:0. This avoids conflicts with shared library version numbers that has been assigned to earlier versions of libctl for Debian; thanks to Josselin Mouette for the suggestion.

libctl 3.0
----------

<small>4/1/2006</small>

-   Switch to use `automake` and `libtool`. Can now install shared libraries with `--enable-shared`.
-   License is now GNU GPL (v2 or later) rather than the GNU LGPL, due to use of third-party GPL code for multi-dimensional integration (below).
-   `gen-ctl-io` now supports separate generation of code and header files via `--code` and `--header` arguments. (Better for parallel make.) Also support a `-o` option to give a different output file name.
-   `gen-ctl-io` can now export C++ code by using the `--cxx` flag.
-   `gen-ctl-io` can now export SWIG `.i` files for automatic type conversion in SWIG wrapper generation, using the `--swig` flag.
-   Backwards incompatible change: users must include their own `ctl-io.h` \*before\* `ctlgeom.h`, or you get `ctlgeom-types.h` instead (this is for use with the "stand-alone" `libctlgeom.a` library below.
-   New multi-dimensional integration routines using adaptive cubature. (Much more efficient than nested 1d integrations.) Adapted in part from the HIntlib Library by Rudolf Schuerer and from the GNU Scientific Library (GSL) by Brian Gough.
-   New `interpolate-uniform` function that tries to maintain a uniform distance between points (i.e. variable number of interpolated points between different list elements, as needed).
-   Now install a "stand-alone" `libctlgeom.a` library to make it easier to call geometry routines from non-Scheme code.
-   New routines to compute overlap fraction of box with object, compute analytical normal vectors, etcetera. (For upcoming versions of Meep and MPB.) Also new routines to get the object of a point, not just the material. Also new routines to operate on a supplied geometry list parameter instead of using the global; unlike the old material_of_point_in_tree functions, these functions do not shift the argument to the unit cell, but you can use the new function shift_to_unit_cell to get this behavior.
-   `gen-ctl-io` now generates object equal/copy functions.
-   In `unit-vector3`, only return 0 when norm==0, not merely if it is small.
-   Added one-sided numerical derivative routine.
-   Define "`verbose?`" variable corresponding to main.c variable.
-   `(print)` calls `(flush-all-ports)` to keep C and Scheme I/O in sync.
-   Fix in `find-root-deriv` to prevent infinite loop in some cases where the root does not exist; thanks to XiuLun Yang for the bug report.
-   Bug fix in `make_hermitian_cmatrix3x3`; thanks to Mischa Megens.

libctl 2.2
----------

<small>9/12/2002</small>

-   Added simple trapezoidal-rule adaptive numeric integration routine.
-   Numerical derivative routines now allow numerical differentation of vector-valued function. Added `deriv2` convenience routine.
-   Added `find-root-deriv` functions for faster root-finding of functions for which the derivative is also available.
-   Added missing `(cvector3` `...)` constructor, and fixed corresponding constructor for `cvector3` object properties; thanks to Doug Allan for the bug report.
-   Added generic '`memoize`' function.
-   libctl programs now print out command-line parameters when they run.
-   Fixed incomplete support for generic `SCM` type.
-   Fixed to work with Guile 1.5+ (thanks to Mike Watts for the bug report).

libctl 2.1
----------

<small>3/21/2002</small>

-   Bug fix: complex-number input variables were read as garbage if they had imaginary parts; does not affect complex-number outputs.
-   Added generic `SCM` type for i/o variables and parameters, as a catch-all for other Scheme objects.
-   `main.c` now has `ctl_export_hook` (enabled by defining `CTL_HAVE_EXPORT_HOOK`) with which to define additional Guile symbols.
-   `gen-ctl-io`: converts "!" in symbols to "B" in C identifiers.

libctl 2.0
----------

<small>3/10/2002</small>

-   New `set-param!` function, analogous to `define-param`, that allows you to change the value of a parameter in a way that can still be overridden from the command line.
-   In libgeom, allow user to specify the resolution instead of the grid-size. New `no-size` support in lattice class to reduce dimensionality, and new `(get-grid-size)` function.
-   Support for Scheme complex numbers, along with a few new associated functions: `conj`, `vector3-cdot`, `matrix3x3-adjoint`.
-   New functions to compute numerical derivatives using Ridder's method of polynomial extrapolation.
-   Documented `object-property-value`; thanks to Theis Peter Hansen for the suggestion.
-   Get rid of unneeded make-default, and use consistent syntax for `define-property` and define-post-processed-property, compared to `define-input-var.` **Not backward compatible** (for developers; users are not affected). Thanks to Theis Peter Hansen for the suggestion.
-   Call `ctl_stop_hook` even with `--help`, `--version`, etcetera; this makes the behavior nicer e.g. with MPI.

libctl 1.5
----------

<small>11/15/2001</small>

-   `geometry-lattice` now has a separate `basis-size` property, so that you can specify the basis vectors as being something other than unit vectors.
-   More functions are tail-recursive, helping to prevent stack overflows; thanks to Robert Sheldon for the bug report.
-   New `fold-left` and `fold-right` functions, documented in the manual.
-   The configure script now checks that `guile` is in the `$PATH`. Thanks to Bing Li and Giridhar Malalahalli for their bug reports.

libctl 1.4.1
------------

<small>7/4/2001</small>

-   Support function lists.

libctl 1.4
----------

<small>2/23/2001</small>

-   Renamed `display-many` function to more felicitous `print` and added `print-ok?` global variable that allows you to disable program output.
-   Added support for passing `'function` types back and forth (just a SCM object pointing to a Scheme function).
-   Cosmetic fixes to yes/no? and menu-choice interaction functions.
-   Support start/exit hooks for use e.g. with MPI.

libctl 1.3
----------

<small>1/7/2001</small>

-   Added improved "subplex" multidimensional optimization algorithm (for `maximize-multiple` and `minimize-multiple`).
-   Documented `vector3-x`, `vector3-y`, `vector3-z` functions for extracting `vector3` components.

libctl 1.2
----------

<small>7/9/2000</small>

-   Added new `cone` geometric object type.
-   Added `reciprocal->cartesian`, `cartesian->lattice`, `lattice->reciprocal`, etcetera functions to libgeom for converting vectors between bases.
-   Added routines `rotate-vector3` and `rotation-matrix3x3` for rotating vectors.
-   Added support for returning lists from external functions.
-   Fixed bug in `matrix3x3-inverse` function.
-   Fixed bug in find-root for converging to negative roots.
-   Use Nelder-Mead simplex algorithm for multi-dimensional minimization (it seems to be more robust than the old routine).

libctl 1.1.1
------------

<small>1/28/2000</small>

-   Use `CPPFLAGS` environment variable instead of the less-standard `INCLUDES` to pass `-I` flags to the configure script (for header files in non-standard locations).
-   Compilation fixes. We need to set `SHELL` in the Makefile for make on some systems. Also added rule to insure `ctl-io.h` is created before main.c is compiled. Thanks to Christoph Becher for the bug reports.

libctl 1.1
----------

<small>1/2/2000</small>

-   geom: radius and height of objects is now permitted to be zero.
-   geom: material_of_point_\* routines now report whether the point is in any object; necessary for use with MPB 0.9.
-   Added man page for `gen-ctl-io`, based on a contribution by James R. Van Zandt.

libctl 1.0.1
------------

<small>11/22/1999</small>

-   geom: handle case where `ensure-periodicity` is false.
-   geom: bug fix in `geometric-objects-lattice-duplicates` for non-orthogonal lattices; thanks to Karl Koch for the bug report.

libctl 1.0
----------

<small>11/19/1999</small>

-   Initial public release.

