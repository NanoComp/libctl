---
# Installation
---

The main effort in installing libctl lies in installing the prerequisite packages. This requires some understanding of how to install software on Unix systems.

The official releases of Libctl can be found on the [releases page on github](https://github.com/NanoComp/libctl/releases), and the changes in each version are summarized in the [NEWS file](https://github.com/NanoComp/libctl/blob/master/NEWS.md).

[TOC]

Installation on Linux
-------------------------

For most [Linux distributions](https://en.wikipedia.org/wiki/Linux_distribution), there should be precompiled packages for most of libctl's prerequisites below, and we *highly* recommend installing those prerequisites using the available packages for your system whenever possible. Using precompiled packages means that you don't have to worry about how to install things manually. You are using packages which have already been tweaked to work well with your system, and usually your packages will be automatically upgraded when you upgrade the rest of your system.

Guile is available as a precompiled package. One thing to be careful of is that many distributions split packages into two parts: one main package for the libraries and programs, and a **devel** package for [header files](https://en.wikipedia.org/wiki/Header_file) and other things needed to compile software using those libraries. You will need to install **both**. So, for example, you will probably need both a `guile` package (probably installed by default) and a `guile-dev` or `guile-devel` package (probably *not* installed by default).

The easiest installation is on [Ubuntu](https://en.wikipedia.org/wiki/Ubuntu_(operating_system)) which has precompiled packages for libctl:

```sh
apt-get install libctl-dev
```

Installation on macOS 
-----------------------

Since [macOS](https://en.wikipedia.org/wiki/macOS) is, at its heart, a Unix system, one can, in principle compile and install libctl and all its prerequisites just as on any other Unix system. However, this process is much easier using the [Homebrew](https://en.wikipedia.org/wiki/Homebrew_(package_management_software)) package to install most of the prerequisites, since it will handle dependencies and other details for you. You will need [administrator privileges](http://support.apple.com/kb/PH3920) on your Mac.

The first steps are:

-   Install [Xcode](https://en.wikipedia.org/wiki/Xcode), the development/compiler package from Apple, free from the [Apple Xcode web page](https://developer.apple.com/xcode/).
-   Install Homebrew: download from the [Homebrew site](http://brew.sh/) and follow the instructions there.
-   Run the following commands in the terminal to compile and install the prerequisites. This may take a while to complete because it will install lots of other stuff first

```sh
brew doctor
brew install guile
```

Now, install libctl from source.

```sh
./configure && make && make install
```

Unix Installation Basics
------------------------

### Installation Paths

First, let's review some important information about installing software on Unix systems, especially in regards to installing software in non-standard locations. None of these issues are specific to libctl, but they've caused a lot of confusion among users.

Most of the software below, including libctl, installs under `/usr/local` by default. That is, libraries go in `/usr/local/lib`, programs in `/usr/local/bin`, etc. If you don't have `root` privileges on your machine, you may need to install somewhere else, e.g. under `$HOME/install` (the `install/` subdirectory of your home directory). Most of the programs below use a GNU-style `configure` script, which means that all you would do to install there would be:

```sh
 ./configure --prefix=$HOME/install
```

when configuring the program. The directories `$HOME/install/lib` etc. are created automatically as needed.

#### Paths for Running (Shared Libraries)

Second, some packages are installed as shared libraries. You need to make sure that your runtime linker knows where to find these shared libraries. The bad news is that every operating system does this in a slightly different way. If you installed all of your libraries in a standard location on your operating system (e.g. `/usr/lib`), then the runtime linker will look there already and you don't need to do anything.  Otherwise, if you compile things like `libctl` and install them into a "nonstandard" location (e.g. in your home directory), you will need to tell the runtime linker where to find them.

There are several ways to do this.  Suppose that you installed libraries into the directory `$HOME/install/lib`. The most robust option is probably to include this path in the linker flags:

```bash
./configure LDFLAGS="-L$HOME/install/lib -Wl,-rpath,$HOME/install/lib"   ...other flags...
```

There are also some other ways.  If you use Linux, have superuser privileges, and are installing in a system-wide location (not your home directory), you can add the library directory to `/etc/ld.so.conf` and run `/sbin/ldconfig`.

On many systems, you can also specify directories to the runtime linker via the `LD_LIBRARY_PATH` environment variable. In particular, by `export LD_LIBRARY_PATH="$HOME/install/lib:$LD_LIBRARY_PATH"`; you can add this to your `.profile` file (depending on your shell) to make it run every time you run your shell. On MacOS, a security feature called [System Integrity Protection](https://en.wikipedia.org/wiki/System_Integrity_Protection) causes the value of `LD_LIBRARY_PATH` to be ignored, so using environment variables won't work there.

### Linux and BSD Binary Packages

If you are installing on your personal Linux or BSD machine, then precompiled binary packages are likely to be available for many of these packages, and may even have been included with your system. On Debian systems, the packages are in `.deb` format and the built-in `apt-get` program can fetch them from a central repository. On Red Hat, SuSE, and most other Linux-based systems, binary packages are in RPM format.  OpenBSD has its "ports" system, and so on.

**Do not compile something from source if an official binary package is available.**  For one thing, you're just creating pain for yourself.  Worse, the binary package may already be installed, in which case installing a different version from source will just cause trouble.

Python Extension for libctlgeom
------

Please follow the [instructions here](../../utils/python/README.md) to install the python extension.

Guile
-----

Guile is an extension/scripting language implementation based on Scheme, and we use it to provide a rich, fully-programmable user interface with minimal effort. It's free, of course, and you can download it from the [Guile homepage](http://www.gnu.org/software/guile/). Guile is typically included with Linux systems.

- **Important:** Most Linux distributions come with Guile already installed. You can check by seeing whether you can run `guile --version` from the command line. In that case, do **not** install your own version of Guile from source &mdash; having two versions of Guile on the same system will cause problems. However, by default most distributions install only the Guile libraries and not the programming headers &mdash; to compile libctl, you should install the **guile-devel** or **guile-dev** package.

It is possible to compile libcti *without* having Guile on your system by passing `--without-guile` to the `configure` script.   (Guile is not necessary to install the Python interface to Meep and MPB.)   However, to do this, you must be building from an official `.tar.gz` package on the [releases page](https://github.com/NanoComp/libctl/releases); building directly from the libctl git repo requires Guile in order to generate certain source files.

### Building From Source

Here we provide instructions for building libctl from source on Ubuntu 16.04. Gzipped tarballs of stable versions are available on the [releases page](https://github.com/NanoComp/libctl/releases).

```bash
#!/bin/bash

set -e

sudo apt-get update
sudo apt-get -y install git guile-2.0-dev

mkdir -p ~/install

cd ~/install
wget https://github.com/NanoComp/libctl/releases/download/v4.2.0/libctl-4.2.0.tar.gz
tar xvzf libctl-4.2.0.tar.gz
cd libctl-4.2.0/
./configure --enable-shared LDFLAGS="-L/usr/local/lib -Wl,-rpath,/usr/local/lib"
make && sudo make install
```

Libctl for Developers
-------------------

If you want to modify the libctl source code, you will want to have a number of additional packages, most importantly:

-   The [Git](https://git-scm.com/) version-control system.

Once you have Git, you can grab the latest development version of libctl with:

```sh
 git clone https://github.com/NanoComp/libctl.git
```

This gives you a fresh, up-to-date libctl repository in a directory `libctl`. See [git-scm.com](https://git-scm.com/) for more information on using Git; perhaps the most useful command is `git pull`, which you can execute periodically to get any new updates to the development version.

Git will give you an absolutely minimal set of sources; to create a usable libctl directory, you should run:

```sh
sh autogen.sh
make
```

in the `libctl` directory. And subsequently, if you are editing the sources you should include `--enable-maintainer-mode` whenever you reconfigure. To do this, however, you will need a number of additional packages beyond those listed above:

-   GNU [autoconf](https://www.gnu.org/software/autoconf/autoconf.html), [automake](https://www.gnu.org/software/automake/), and [libtool](https://www.gnu.org/software/libtool/libtool.html) &mdash; these are used to create the Makefiles and configure scripts, and to build shared libraries.
