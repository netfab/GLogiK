
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
		%D%/LCDScreenPluginsManager.cpp \
		%D%/LCDScreenPluginsManager.hpp

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
		%D%/clientsManager.cpp \
		%D%/clientsManager.hpp \
		%D%/clientsSignals.cpp \
		%D%/clientsSignals.hpp \
		%D%/client.cpp \
		%D%/client.hpp \
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
		%D%/Vendor/Logitech/G510/G510.hpp \
		%D%/Vendor/Logitech/G510/G510Base.cpp \
		%D%/Vendor/Logitech/G510/G510Base.hpp \
		%D%/Vendor/Logitech/G510/G510Detail.hpp

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

