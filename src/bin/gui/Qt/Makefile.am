
bin_PROGRAMS += GKcQt5

GKcQt5_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@QT5_CFLAGS@ \
				@DBUS_CFLAGS@ \
				@AM_CXXFLAGS@

GKcQt5_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GKcQt5_LDADD = \
		@QT5_LIBS@ \
		@BOOST_SYSTEM_LIB@ \
		@BOOST_FILESYSTEM_LIB@ \
		@BOOST_PROGRAM_OPTIONS_LIB@ \
		libGKDBus.la \
		libGKUtils.la \
		libGKShared.la

GKcQt5_SOURCES = \
		%D%/AboutDialog/AboutDialogTab.hpp \
		%D%/AboutDialog/AboutTab.cpp \
		%D%/AboutDialog/AboutTab.hpp \
		%D%/AboutDialog/DependenciesTab.cpp \
		%D%/AboutDialog/DependenciesTab.hpp \
		%D%/AboutDialog/LicenseTab.cpp \
		%D%/AboutDialog/LicenseTab.hpp \
		%D%/AboutDialog/AboutDialog.cpp \
		%D%/AboutDialog/AboutDialog.hpp \
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
