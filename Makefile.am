
ACLOCAL_AMFLAGS = -I m4

AM_DISTCHECK_CONFIGURE_FLAGS = --enable-gcc-wall --enable-gcc-werror --enable-as-needed \
	--enable-dbus --enable-qt5

include %D%/data/init/openrc/Makefile.am
include %D%/data/udev/Makefile.am
include %D%/data/pbm/Makefile.am

include %D%/src/include/Makefile.am
include %D%/src/lib/utils/Makefile.am
include %D%/src/lib/shared/Makefile.am

if WITH_DBUS

include %D%/data/autostart/Makefile.am
include %D%/data/dbus/Makefile.am
include %D%/src/lib/dbus/Makefile.am

include %D%/src/bin/launcher/Makefile.am
include %D%/src/bin/service/Makefile.am

if WITH_QT5
include %D%/src/bin/gui/Qt/Makefile.am
include %D%/data/applications/Makefile.am
include %D%/data/icons/Makefile.am
endif

endif

include %D%/src/bin/daemon/Makefile.am


maintainer-clean-local: maintainer-clean-libtool-files maintainer-clean-autotools-files

.PHONY: maintainer-clean-libtool-files maintainer-clean-autotools-files

LOCAL_LIBTOOL_FILES = \
	$(top_srcdir)/m4/libtool.m4 \
	$(top_srcdir)/m4/ltoptions.m4 \
	$(top_srcdir)/m4/ltsugar.m4 \
	$(top_srcdir)/m4/ltversion.m4 \
	$(top_srcdir)/m4/lt~obsolete.m4

LOCAL_AUTOTOOLS_FILES = \
	$(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/config.h.in \
	$(top_srcdir)/depcomp \
	$(top_srcdir)/install-sh \
	$(top_srcdir)/missing \
	$(top_srcdir)/configure \
	$(top_srcdir)/config.h.in~ \
	$(top_srcdir)/config.guess \
    $(top_srcdir)/config.sub \
	$(top_srcdir)/compile \
	$(top_srcdir)/ltmain.sh

maintainer-clean-libtool-files:
	-rm -f $(LOCAL_LIBTOOL_FILES)

maintainer-clean-autotools-files:
	-rm -f $(LOCAL_AUTOTOOLS_FILES)


NEWS.md:

README.md:

NEWS: NEWS.md
	-cp $< $@

README: README.md
	-cp $< $@

CLEANFILES = NEWS README
