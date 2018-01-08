
sbin_PROGRAMS = GLogiKd

GLogiKd_CXXFLAGS = \
				@LIBEVDEV_CFLAGS@ \
				@LIBUSB_CFLAGS@ \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@ \
				@PTHREAD_CFLAGS@ \
				@AM_CXXFLAGS@

GLogiKd_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GLogiKd_LDADD = \
				@LIBEVDEV_LIBS@ \
				@LIBUSB_LIBS@ \
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@BOOST_IOSTREAMS_LIB@ \
				@LIBUDEV_LIBS@ \
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
		clientsSignals.cpp clientsSignals.h \
		client.cpp client.h

