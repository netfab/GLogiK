
bin_PROGRAMS = GKsLauncher

GKsLauncher_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@DBUS_CFLAGS@ \
				@AM_CXXFLAGS@

GKsLauncher_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GKsLauncher_LDADD = \
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				libGKDBus.la \
				libGKUtils.la \
				libGKShared.la

GKsLauncher_SOURCES = \
		%D%/main.cpp \
		%D%/DBusHandler.cpp \
		%D%/DBusHandler.hpp \
		%D%/launcher.cpp \
		%D%/launcher.hpp
