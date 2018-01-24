
bin_PROGRAMS = GLogiKs

GLogiKs_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@ \
				@AM_CXXFLAGS@

GLogiKs_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GLogiKs_LDADD = \
				@BOOST_SERIALIZATION_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				libGLogiKDBus.la \
				libGLogiKUtils.la \
				libGLogiKShared.la

GLogiKs_SOURCES = \
		%D%/main.cpp \
		%D%/service.cpp \
		%D%/service.h \
		%D%/serviceDBusHandler.cpp \
		%D%/serviceDBusHandler.h \
		%D%/devicesHandler.cpp \
		%D%/devicesHandler.h \
		%D%/warningCheck.cpp \
		%D%/warningCheck.h
