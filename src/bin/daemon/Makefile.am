
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
		%D%/LCD/pluginsManager.cpp \
		%D%/LCD/pluginsManager.hpp

if WITH_HIDAPI
GLogiKd_CXXFLAGS += \
				@LIBHIDAPI_CFLAGS@

GLogiKd_LDADD += \
				@LIBHIDAPI_LIBS@

GLogiKd_SOURCES += \
		%D%/USB/API/hidapi.cpp \
		%D%/USB/API/hidapi.hpp
else
GLogiKd_SOURCES += \
		%D%/USB/API/libusb.cpp \
		%D%/USB/API/libusb.hpp
endif

if WITH_DBUS
GLogiKd_CXXFLAGS += \
				@DBUS_CFLAGS@

GLogiKd_LDADD += \
				libGKDBus.la

GLogiKd_SOURCES += \
		%D%/Clients/client.cpp \
		%D%/Clients/client.hpp \
		%D%/Clients/clientsManager.cpp \
		%D%/Clients/clientsManager.hpp \
		%D%/Clients/clientsSignals.cpp \
		%D%/Clients/clientsSignals.hpp \
		%D%/sleepInhibition.cpp \
		%D%/sleepInhibition.hpp
endif

GLogiKd_SOURCES += \
		%D%/USB/API/detail.hpp \
		%D%/USB/API/usbinit.cpp \
		%D%/USB/API/usbinit.hpp \
		%D%/USB/Device/detail.hpp \
		%D%/USB/Device/USBDevice.cpp \
		%D%/USB/Device/USBDevice.hpp \
		%D%/USB/Device/USBDeviceID.cpp \
		%D%/USB/Device/USBDeviceID.hpp \
		%D%/USB/Keyboard/KeyboardDriver.cpp \
		%D%/USB/Keyboard/KeyboardDriver.hpp \
		%D%/USB/Keyboard/KeyboardDriverDetail.hpp \
		%D%/USB/Keyboard/USBKeyboardDriver.hpp \
		%D%/Vendor/Logitech/G510/detail.hpp \
		%D%/Vendor/Logitech/G510/G510.hpp \
		%D%/Vendor/Logitech/G510/G510Base.cpp \
		%D%/Vendor/Logitech/G510/G510Base.hpp

GLogiKd_SOURCES += \
		%D%/LCD/Plugins/PBM.hpp \
		%D%/LCD/Plugins/PBMFile.cpp \
		%D%/LCD/Plugins/PBMFile.hpp \
		%D%/LCD/Plugins/PBMFont.cpp \
		%D%/LCD/Plugins/PBMFont.hpp \
		%D%/LCD/Plugins/fonts.hpp \
		%D%/LCD/Plugins/fontsManager.cpp \
		%D%/LCD/Plugins/fontsManager.hpp \
		%D%/LCD/Plugins/LCDPlugin.cpp \
		%D%/LCD/Plugins/LCDPlugin.hpp \
		%D%/LCD/Plugins/splashscreen.cpp \
		%D%/LCD/Plugins/splashscreen.hpp \
		%D%/LCD/Plugins/cpu-stats/CPUData.cpp \
		%D%/LCD/Plugins/cpu-stats/CPUData.h \
		%D%/LCD/Plugins/cpu-stats/CPUSnapshot.cpp \
		%D%/LCD/Plugins/cpu-stats/CPUSnapshot.h \
		%D%/LCD/Plugins/netsnap/netSnapshots.cpp \
		%D%/LCD/Plugins/netsnap/netSnapshots.hpp \
		%D%/LCD/Plugins/systemMonitor.cpp \
		%D%/LCD/Plugins/systemMonitor.hpp \
		%D%/LCD/Plugins/coretemp.cpp \
		%D%/LCD/Plugins/coretemp.hpp \
		%D%/LCD/Plugins/endscreen.cpp \
		%D%/LCD/Plugins/endscreen.hpp

