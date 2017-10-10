
bin_PROGRAMS = GLogiKs

GLogiKs_CXXFLAGS= @AM_CXXFLAGS@ \
				@DBUS_CFLAGS@

GLogiKs_LDFLAGS= @AM_LDFLAGS@

GLogiKs_LDADD= \
				../../lib/dbus/libGLogiKDBus.la \
				../../lib/utils/libGLogiKUtils.la \
				../../lib/shared/libGLogiKShared.la

GLogiKs_SOURCES = \
		main.cpp \
		service.cpp service.h \
		serviceDBusHandler.cpp serviceDBusHandler.h
