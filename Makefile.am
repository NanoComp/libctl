SUBDIRS = src utils examples
EXTRA_DIST = COPYRIGHT doc base

BASE_SCM = base/class.scm base/ctl.scm base/extern-funcs.scm		\
base/help.scm base/include.scm base/interaction.scm base/io-vars.scm	\
base/math-utils.scm base/matrix3x3.scm base/simplex.scm base/utils.scm	\
base/vector3.scm

nobase_dist_pkgdata_DATA = $(BASE_SCM) base/main.c utils/ctl-io.scm utils/geom.c utils/geom.scm

dist-hook:
	darcs changes --summary > $(distdir)/ChangeLog
