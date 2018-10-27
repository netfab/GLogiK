
bin_PROGRAMS += GKcQt5

### QT_CXXFLAGS adds '-W', which is the old name of '-Wextra'
GKcQt5_CXXFLAGS = \
				@QT_CXXFLAGS@ \
				-Wno-extra \
				@DBUS_CFLAGS@ \
				@AM_CXXFLAGS@

GKcQt5_LDFLAGS = \
				@AM_LDFLAGS@

GKcQt5_LDADD = \
		@QT_LIBS@ \
		@BOOST_SYSTEM_LIB@ \
		@BOOST_FILESYSTEM_LIB@ \
		libGKDBus.la \
		libGKUtils.la \
		libGKShared.la

GKcQt5_SOURCES = \
		%D%/DeviceControlTab.cpp \
		%D%/DeviceControlTab.hpp \
		%D%/BacklightColorTab.cpp \
		%D%/BacklightColorTab.hpp \
		%D%/mainWindow.cpp \
		%D%/mainWindow.hpp \
		%D%/main.cpp
