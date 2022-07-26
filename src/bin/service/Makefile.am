
bin_PROGRAMS += GLogiKs

GLogiKs_CXXFLAGS = \
				@LIBEVDEV_CFLAGS@ \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@ \
				@PTHREAD_CFLAGS@ \
				@AM_CXXFLAGS@ \
				@LIBX11_CFLAGS@ \
				@LIBXTST_CFLAGS@

GLogiKs_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GLogiKs_LDADD = \
				@LIBEVDEV_LIBS@ \
				@BOOST_SERIALIZATION_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				@PTHREAD_LIBS@ \
				@LIBX11_LIBS@ \
				@LIBXTST_LIBS@ \
				libGKDBus.la \
				libGKUtils.la \
				libGKShared.la

GLogiKs_SOURCES = \
		%D%/main.cpp \
		%D%/devicesHandler.cpp \
		%D%/devicesHandler.hpp \
		%D%/DBusHandler.cpp \
		%D%/DBusHandler.hpp \
		%D%/service.cpp \
		%D%/service.hpp

