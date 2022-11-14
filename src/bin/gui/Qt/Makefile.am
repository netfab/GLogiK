
bin_PROGRAMS += GKcQt5

### QT_CXXFLAGS adds '-W', which is the old name of '-Wextra'
GKcQt5_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@QT_CXXFLAGS@ \
				-Wno-extra \
				@DBUS_CFLAGS@ \
				@AM_CXXFLAGS@

GKcQt5_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GKcQt5_LDADD = \
		@QT_LIBS@ \
		@BOOST_SYSTEM_LIB@ \
		@BOOST_FILESYSTEM_LIB@ \
		@BOOST_PROGRAM_OPTIONS_LIB@ \
		libGKDBus.la \
		libGKUtils.la \
		libGKShared.la

GKcQt5_SOURCES = \
		%D%/AboutDialog.cpp \
		%D%/AboutDialog.hpp \
		%D%/Tab.cpp \
		%D%/Tab.hpp \
		%D%/DaemonAndServiceTab.cpp \
		%D%/DaemonAndServiceTab.hpp \
		%D%/DeviceControlTab.cpp \
		%D%/DeviceControlTab.hpp \
		%D%/BacklightColorTab.cpp \
		%D%/BacklightColorTab.hpp \
		%D%/LCDPluginsTab.cpp \
		%D%/LCDPluginsTab.hpp \
		%D%/GKeysTab.cpp \
		%D%/GKeysTab.hpp \
		%D%/mainWindow.cpp \
		%D%/mainWindow.hpp \
		%D%/main.cpp
