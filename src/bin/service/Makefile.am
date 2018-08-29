
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
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				@PTHREAD_LIBS@ \
				libGKDBus.la \
				libGKUtils.la \
				libGKShared.la

GLogiKs_SOURCES = \
		%D%/main.cpp \
		%D%/global.hpp \
		%D%/devicesHandler.cpp \
		%D%/devicesHandler.hpp \
		%D%/DBusHandler.cpp \
		%D%/DBusHandler.hpp \
		%D%/service.cpp \
		%D%/service.hpp

if WITH_LIBNOTIFY

GLogiKs_CXXFLAGS += @LIBNOTIFY_CFLAGS@
GLogiKs_LDADD += @LIBNOTIFY_LIBS@
GLogiKs_SOURCES += \
	%D%/volumeNotification.cpp \
	%D%/volumeNotification.hpp

endif
