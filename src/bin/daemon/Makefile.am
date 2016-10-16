
bin_PROGRAMS = GLogiKd

GLogiKd_CXXFLAGS= @AM_CXXFLAGS@ @BOOST_CPPFLAGS@
GLogiKd_LDFLAGS= @AM_LDFLAGS@ @BOOST_LDFLAGS@
GLogiKd_LDADD= @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_SYSTEM_LIB@ @BOOST_FILESYSTEM_LIB@ @BOOST_ATOMIC_LIB@ @LIBUDEV_LIBS@

GLogiKd_SOURCES = \
		main.cpp \
		daemon.cpp daemon.h \
		devices_manager.cpp devices_manager.h \
		logitech_g15.cpp logitech_g15.h \
		keyboard_driver.cpp keyboard_driver.h \
		exception.cpp exception.h \
		globals.h

