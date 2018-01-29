#!/bin/sh

mkdir -p m4

# paranoia: sometimes autoconf doesn't get things right the first time
autoreconf --verbose --install --symlink --force
autoreconf --verbose --install --symlink --force
autoreconf --verbose --install --symlink --force

./configure --enable-maintainer-mode "$@"
