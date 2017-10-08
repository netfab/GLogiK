
sbin_PROGRAMS = GLogiKd

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
				../../lib/utils/libGLogiKUtils.la \
				../../lib/dbus/libGLogiKDBus.la \
				../../lib/shared/libGLogiKShared.la

GLogiKd_SOURCES = \
		main.cpp \
		daemon.cpp daemon.h \
		daemonControl.cpp daemonControl.h \
		devicesManager.cpp devicesManager.h \
		keyboardDriver.cpp keyboardDriver.h \
		virtualKeyboard.cpp virtualKeyboard.h \
		logitechG510.cpp logitechG510.h \
		macrosManager.cpp macrosManager.h \
		clientsManager.cpp clientsManager.h \
		client.cpp client.h

