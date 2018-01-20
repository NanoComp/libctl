SUBDIRS = utils
if WITH_GUILE
SUBDIRS += src examples
endif
EXTRA_DIST = COPYRIGHT doc base README.md NEWS.md
ACLOCAL_AMFLAGS = -I m4

BASE_SCM = base/class.scm base/ctl.scm base/extern-funcs.scm		\
base/help.scm base/include.scm base/interaction.scm base/io-vars.scm	\
base/math-utils.scm base/matrix3x3.scm base/simplex.scm base/utils.scm	\
base/vector3.scm

nobase_dist_pkgdata_DATA = $(BASE_SCM) base/main.c utils/ctl-io.scm utils/geom.c utils/geom.scm utils/nlopt-constants.scm utils/nlopt.c
