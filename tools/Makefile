SHELL = /bin/sh
top_srcdir = ..
build_dir= ../build
PREFIX_DEV_DIR=/var/tmp/devcxx
PREFIX_DIR=$(PREFIX_DEV_DIR)/GLogiK
LANG=C
CXXFLAGS="-march=sandybridge -mtune=generic -O2 -pipe -mmmx -msse -msse2"

#MACROFILES= libtool.m4 ltoptions.m4 ltsugar.m4 ltversion.m4 lt~obsolete.m4
#	cd $(top_srcdir)/m4 && rm -v $(MACROFILES)

#    config.guess \
#    config.sub \
#    ltmain.sh \

MAINTAINERCLEANFILES = \
    aclocal.m4 \
    config.h.in \
    depcomp \
    install-sh \
    missing \
    configure \
    config.guess \
    config.sub \
    compile

all:
	@echo "Available targets :"
	@echo " autoreconf_it configure_it build_it install_it distcheck_it"
	@echo " uninstall_it clean_it fullclean_it init_dir all"
	@echo

init_dir:
	test ! -d $(build_dir) && mkdir -p $(build_dir) || echo "build_dir already present"

autoreconf_it:
	cd $(top_srcdir) && autoreconf -i


configure_it: init_dir
	cd $(build_dir) && $(top_srcdir)/configure CXXFLAGS=$(CXXFLAGS) \
	--enable-debug DEBUG_DIR=$(PREFIX_DEV_DIR) DEBUG_LOG_LEVEL=DEBUG4 \
	--enable-gcc-wall --enable-gcc-werror --enable-as-needed \
	--srcdir=$(top_srcdir) --prefix=$(PREFIX_DIR)

build_it:
	cd $(build_dir) && make

install_it:
	find $(PREFIX_DEV_DIR) -type f -name 'glogikd-debug-*.log' -delete
	cd $(build_dir) && make install

uninstall_it:
	cd $(build_dir) && test -d "$(PREFIX_DIR)" && make uninstall || echo 'no need uninstall phase.'
	cd "$(PREFIX_DEV_DIR)" && rmdir -v -p GLogiK/bin || echo 'uninstall artefacts rmdir failure.'

clean_it: uninstall_it
	cd $(build_dir) && make clean

fullclean_it: clean_it
	cd $(build_dir) && make maintainer-clean
	cd $(top_srcdir) && rm -v $(MAINTAINERCLEANFILES)
	find $(top_srcdir) -name 'Makefile.in' -delete
	cd $(build_dir) && find . -depth -type d -delete
	cd $(build_dir) && test -f *.tar.gz && rm -v *.tar.gz || echo 'archive not found.'
	rmdir -v $(build_dir)

distcheck_it:
	cd $(build_dir) && make distcheck

