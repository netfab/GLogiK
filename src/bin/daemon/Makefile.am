
bin_PROGRAMS = GLogiKd

GLogiKd_CXXFLAGS= @AM_CXXFLAGS@ \
				@BOOST_CPPFLAGS@ \
				@LIBUSB_CFLAGS@ \
				@LIBEVDEV_CFLAGS@ \
				@PTHREAD_CFLAGS@ \
				@DBUS_CFLAGS@

GLogiKd_LDFLAGS= @AM_LDFLAGS@ @BOOST_LDFLAGS@

GLogiKd_LDADD=	@LIBUDEV_LIBS@ \
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@BOOST_IOSTREAMS_LIB@ \
				@LIBUSB_LIBS@ \
				@LIBEVDEV_LIBS@ \
				@PTHREAD_LIBS@ \
				../../lib/dbus/libGLogiKDBus.la

GLogiKd_SOURCES = \
		main.cpp \
		daemon.cpp daemon.h \
		daemon_control.cpp daemon_control.h \
		devices_manager.cpp devices_manager.h \
		keyboard_driver.cpp keyboard_driver.h \
		virtual_keyboard.cpp virtual_keyboard.h \
		logitech_G510.cpp logitech_G510.h \
		exception.cpp exception.h \
		globals.h key_event.h \
		macros_manager.cpp macros_manager.h

