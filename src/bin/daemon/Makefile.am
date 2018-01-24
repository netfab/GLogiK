
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
				libGLogiKUtils.la \
				libGLogiKDBus.la \
				libGLogiKShared.la

GLogiKd_SOURCES = \
		%D%/main.cpp \
		%D%/daemon.cpp \
		%D%/daemon.h \
		%D%/daemonControl.cpp \
		%D%/daemonControl.h \
		%D%/devicesManager.cpp \
		%D%/devicesManager.h \
		%D%/keyboardDriver.cpp \
		%D%/keyboardDriver.h \
		%D%/virtualKeyboard.cpp \
		%D%/virtualKeyboard.h \
		%D%/logitechG510.cpp \
		%D%/logitechG510.h \
		%D%/macrosManager.cpp \
		%D%/macrosManager.h \
		%D%/clientsManager.cpp \
		%D%/clientsManager.h \
		%D%/clientsSignals.cpp \
		%D%/clientsSignals.h \
		%D%/client.cpp \
		%D%/client.h

