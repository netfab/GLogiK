
sbin_PROGRAMS = GLogiKd

GLogiKd_CXXFLAGS = \
				@BOOST_CPPFLAGS@ \
				@LIBUSB_CFLAGS@ \
				@PTHREAD_CFLAGS@ \
				@AM_CXXFLAGS@

GLogiKd_LDFLAGS = \
				@BOOST_LDFLAGS@ \
				@AM_LDFLAGS@

GLogiKd_LDADD = \
				@BOOST_PROGRAM_OPTIONS_LIB@ \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@LIBUDEV_LIBS@ \
				@LIBUSB_LIBS@ \
				@PTHREAD_LIBS@ \
				libGKUtils.la \
				libGKShared.la

GLogiKd_SOURCES = \
		%D%/main.cpp \
		%D%/daemon.cpp \
		%D%/daemon.hpp \
		%D%/daemonControl.cpp \
		%D%/daemonControl.hpp \
		%D%/devicesManager.cpp \
		%D%/devicesManager.hpp \
		%D%/USBAPIenums.hpp \
		%D%/keyboardDriver.cpp \
		%D%/keyboardDriver.hpp \
		%D%/usbinit.cpp \
		%D%/usbinit.hpp \
		%D%/USBDevice.cpp \
		%D%/USBDevice.hpp \
		%D%/USBDeviceID.cpp \
		%D%/USBDeviceID.hpp \
		%D%/logitechG510.cpp \
		%D%/logitechG510.hpp \
		%D%/LCDScreenPluginsManager.cpp \
		%D%/LCDScreenPluginsManager.hpp

if WITH_HIDAPI
GLogiKd_CXXFLAGS += \
				@LIBHIDAPI_CFLAGS@

GLogiKd_LDADD += \
				@LIBHIDAPI_LIBS@

GLogiKd_SOURCES += \
		%D%/hidapi.cpp \
		%D%/hidapi.hpp
else
GLogiKd_SOURCES += \
		%D%/libusb.cpp \
		%D%/libusb.hpp
endif

if WITH_DBUS
GLogiKd_CXXFLAGS += \
				@DBUS_CFLAGS@

GLogiKd_LDADD += \
				libGKDBus.la

GLogiKd_SOURCES += \
		%D%/clientsManager.cpp \
		%D%/clientsManager.hpp \
		%D%/clientsSignals.cpp \
		%D%/clientsSignals.hpp \
		%D%/client.cpp \
		%D%/client.hpp
endif

GLogiKd_SOURCES += \
		%D%/LCDPlugins/PBM.hpp \
		%D%/LCDPlugins/PBMFile.cpp \
		%D%/LCDPlugins/PBMFile.hpp \
		%D%/LCDPlugins/PBMFont.cpp \
		%D%/LCDPlugins/PBMFont.hpp \
		%D%/LCDPlugins/fonts.hpp \
		%D%/LCDPlugins/fontsManager.cpp \
		%D%/LCDPlugins/fontsManager.hpp \
		%D%/LCDPlugins/LCDPlugin.cpp \
		%D%/LCDPlugins/LCDPlugin.hpp \
		%D%/LCDPlugins/splashscreen.cpp \
		%D%/LCDPlugins/splashscreen.hpp \
		%D%/LCDPlugins/cpu-stats/CPUData.cpp \
		%D%/LCDPlugins/cpu-stats/CPUData.h \
		%D%/LCDPlugins/cpu-stats/CPUSnapshot.cpp \
		%D%/LCDPlugins/cpu-stats/CPUSnapshot.h \
		%D%/LCDPlugins/netsnap/netSnapshots.cpp \
		%D%/LCDPlugins/netsnap/netSnapshots.hpp \
		%D%/LCDPlugins/systemMonitor.cpp \
		%D%/LCDPlugins/systemMonitor.hpp \
		%D%/LCDPlugins/coretemp.cpp \
		%D%/LCDPlugins/coretemp.hpp \
		%D%/LCDPlugins/endscreen.cpp \
		%D%/LCDPlugins/endscreen.hpp

