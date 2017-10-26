
bin_PROGRAMS = GLogiKs

GLogiKs_CXXFLAGS= @AM_CXXFLAGS@ \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@

GLogiKs_LDFLAGS= @AM_LDFLAGS@ \
				@BOOST_LDFLAGS@

GLogiKs_LDADD= @BOOST_SERIALIZATION_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				../../lib/dbus/libGLogiKDBus.la \
				../../lib/utils/libGLogiKUtils.la \
				../../lib/shared/libGLogiKShared.la

GLogiKs_SOURCES = \
		main.cpp \
		service.cpp service.h \
		serviceDBusHandler.cpp serviceDBusHandler.h \
		devicesHandler.cpp devicesHandler.h \
		warningCheck.cpp warningCheck.h
