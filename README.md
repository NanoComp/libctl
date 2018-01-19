[![Latest Docs](https://readthedocs.org/projects/pip/badge/?version=latest)](http://libctl.readthedocs.io/en/latest/)
[![Build Status](https://travis-ci.org/stevengj/libctl.svg?branch=master)](https://travis-ci.org/stevengj/libctl)

This is libctl, a [Guile](http://www.gnu.org/software/guile/)-based library for supporting flexible control
files in scientific simulations.

To install libctl, one normally only needs to do:

    ./configure
    make
    make install

Files are installed under `/usr/local` by default, but this can be
changed by passing `--prefix=<dir>` to `configure`.

Documentation can be found in [ReadTheDocs](https://libctl.readthedocs.io) or in the `doc/` directory, and an example
program in the `examples/ directory`.  The main source code for libctl
is in the `base/` and `utils/` directories.

In `utils/geom.*`, you can find specification files and functions for
dealing with structures consisting of solid geometric objects in some
lattice basis.  The example program uses this code.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
