SHELL = /bin/sh
top_srcdir = $(CURDIR)/..
PREFIX_DEV_DIR=/var/tmp/devcxx

build_dir=$(PREFIX_DEV_DIR)/build
PREFIX_DIR=$(PREFIX_DEV_DIR)/GLogiK

LANG=C

CXXFLAGS=-O2 -pipe \
	-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
	-Wformat=2

#
# CXXFLAGS="-march=sandybridge -mtune=generic -O2 -pipe -mmmx -msse -msse2"
# CXXFLAGS="-march=sandybridge -mtune=generic -O2 -pipe -mmmx -msse -msse2 -I$(PREFIX_DEV_DIR)/libg15/include"
#
# XXX
# Link with libg15 in custom path - configure phase
#
#		CXXFLAGS=$(CXXFLAGS) \
#		LDFLAGS="-L$(PREFIX_DEV_DIR)/libg15/lib -Wl,-rpath -Wl,$(PREFIX_DEV_DIR)/libg15/lib" \

ifeq ($(strip $(DEBUG_BUILD)),)
all:
	@echo "Available targets :"
	@echo "-------------------"
	@echo " (debug off, stripped build)"
	@echo "----------------------------"
	@echo " configure_it build_it install_it distcheck_it"
	@echo " uninstall_it clean_it fullclean_it init_it all"
	@echo
else
all:
	@echo "Available targets :"
	@echo "-------------------"
	@echo " (debug on, unstripped build)"
	@echo "-----------------------------"
	@echo " configure_it build_it install_it distcheck_it"
	@echo " uninstall_it clean_it fullclean_it init_it"
	@echo
endif

# -- -- --
# -- -- --
# -- -- --

init_it:
	test ! -d $(build_dir) && mkdir -p $(build_dir) || echo "build_dir already present"

build_it:
	cd $(build_dir) && make -j3

uninstall_it:
	cd $(build_dir) && test -d "$(PREFIX_DIR)" && make uninstall || echo 'no need uninstall phase.'
	test -d "$(PREFIX_DIR)" && \
		cd "$(PREFIX_DEV_DIR)" && find GLogiK -depth -type d -print -empty -delete || echo 'removing empty directories failed'

clean_it: uninstall_it
	cd $(build_dir) && make clean

autoreconf_it:
	cd $(top_srcdir) && autoreconf -i

fullclean_it: clean_it
	cd $(build_dir) && make maintainer-clean
	find $(top_srcdir) -name 'Makefile.in' -delete
	cd $(build_dir) && find . -depth -type d -delete
	cd $(build_dir) && test -f *.tar.gz && rm -v *.tar.gz || echo 'archive not found.'
	rmdir -v $(build_dir)

distcheck_it:
	cd $(build_dir) && make -j3 distcheck

# -- -- --
# -- -- --
# -- -- --


ifeq ($(strip $(DEBUG_BUILD)),)

# -- -- --
# -- -- -- DEBUG OFF, STRIPPED BUILD
# -- -- --

configure_it: init_it autoreconf_it
	cd $(build_dir) && $(top_srcdir)/configure \
	CXXFLAGS="$(CXXFLAGS)" \
	--disable-debug --disable-debug-gkdbus  --disable-debug-libusb \
	--disable-debug-pbmfont --disable-debug-lcd-plugins \
	DEBUG_DIR=$(PREFIX_DEV_DIR) \
	--enable-gcc-wall --enable-gcc-werror --enable-as-needed \
	--srcdir=$(top_srcdir) --prefix=$(PREFIX_DIR)

install_it: uninstall_it
	cd $(build_dir) && make install-strip

else

# -- -- --
# -- -- -- DEBUG ON, UNSTRIPPED
# -- -- --

configure_it: init_it autoreconf_it
	cd $(build_dir) && $(top_srcdir)/configure \
	CXXFLAGS="$(CXXFLAGS)" \
	--enable-debug --disable-debug-gkdbus --disable-debug-libusb \
	--disable-debug-pbmfont --disable-debug-lcd-plugins \
	DEBUG_DIR=$(PREFIX_DEV_DIR) \
	--enable-gcc-wall --enable-gcc-werror --enable-as-needed \
	--enable-desktop-notifications \
	--srcdir=$(top_srcdir) --prefix=$(PREFIX_DIR)

install_it: uninstall_it
	find $(PREFIX_DEV_DIR) -type f -name 'G*-debug-*.log' -delete
	cd $(build_dir) && make install

endif

# -- -- --
# -- -- --
# -- -- --

