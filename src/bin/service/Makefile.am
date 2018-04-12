
bin_PROGRAMS = GLogiKs

GLogiKs_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@ \
				@PTHREAD_CFLAGS@ \
				@AM_CXXFLAGS@

GLogiKs_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GLogiKs_LDADD = \
				@BOOST_SERIALIZATION_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@PTHREAD_LIBS@ \
				libGKDBus.la \
				libGKUtils.la \
				libGKShared.la

GLogiKs_SOURCES = \
		%D%/main.cpp \
		%D%/service.cpp \
		%D%/service.h \
		%D%/serviceDBusHandler.cpp \
		%D%/serviceDBusHandler.h \
		%D%/devicesHandler.cpp \
		%D%/devicesHandler.h
