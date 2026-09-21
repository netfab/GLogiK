
GLogiKd_sources = [
  'main.cpp',
  'daemon.cpp',
  'daemon.hpp',
  'daemonControl.cpp',
  'daemonControl.hpp',
  'devicesManager.cpp',
  'devicesManager.hpp',
  'LCD/pluginsManager.cpp',
  'LCD/pluginsManager.hpp',
]

GLogiKd_sources += [
  'USB/API/detail.hpp',
  'USB/API/usbinit.cpp',
  'USB/API/usbinit.hpp',
  'USB/Device/detail.hpp',
  'USB/Device/USBDevice.cpp',
  'USB/Device/USBDevice.hpp',
  'USB/Device/USBDeviceID.cpp',
  'USB/Device/USBDeviceID.hpp',
  'USB/Keyboard/detail.hpp',
  'USB/Keyboard/KeyboardDriver.cpp',
  'USB/Keyboard/KeyboardDriver.hpp',
  'USB/Keyboard/USBKeyboardDriver.hpp',
  'Vendor/Logitech/G510/detail.hpp',
  'Vendor/Logitech/G510/G510.hpp',
  'Vendor/Logitech/G510/G510Base.cpp',
  'Vendor/Logitech/G510/G510Base.hpp',
]

GLogiKd_sources += [
  'LCD/Plugins/PBM.hpp',
  'LCD/Plugins/PBMFile.cpp',
  'LCD/Plugins/PBMFile.hpp',
  'LCD/Plugins/PBMFont.cpp',
  'LCD/Plugins/PBMFont.hpp',
  'LCD/Plugins/fonts.hpp',
  'LCD/Plugins/fontsManager.cpp',
  'LCD/Plugins/fontsManager.hpp',
  'LCD/Plugins/LCDPlugin.cpp',
  'LCD/Plugins/LCDPlugin.hpp',
  'LCD/Plugins/splashscreen.cpp',
  'LCD/Plugins/splashscreen.hpp',
  'LCD/Plugins/cpu-stats/CPUData.cpp',
  'LCD/Plugins/cpu-stats/CPUData.h',
  'LCD/Plugins/cpu-stats/CPUSnapshot.cpp',
  'LCD/Plugins/cpu-stats/CPUSnapshot.h',
  'LCD/Plugins/netsnap/netSnapshots.cpp',
  'LCD/Plugins/netsnap/netSnapshots.hpp',
  'LCD/Plugins/systemMonitor.cpp',
  'LCD/Plugins/systemMonitor.hpp',
  'LCD/Plugins/coretemp.cpp',
  'LCD/Plugins/coretemp.hpp',
  'LCD/Plugins/endscreen.cpp',
  'LCD/Plugins/endscreen.hpp',
]

if enable_dbus
  GLogiKd_sources += [
    'Clients/client.cpp',
    'Clients/client.hpp',
    'Clients/clientsManager.cpp',
    'Clients/clientsManager.hpp',
    'Clients/clientsSignals.cpp',
    'Clients/clientsSignals.hpp',
    'sleepInhibition.cpp',
    'sleepInhibition.hpp',
  ]
endif

#! ----------

libusb_dep = dependency('libusb-1.0', version: '>=1.0.19')
hidapi_dep = null_dep
if enable_hidapi
  hidapi_dep = dependency('hidapi-libusb', version: '>=0.10.0')
endif

if hidapi_dep.found()
  GLogiKd_sources += [
    'USB/API/hidapi.cpp',
    'USB/API/hidapi.hpp',
  ]
  config_h.set('HAVE_HIDAPI', 1)
elif libusb_dep.found()
  GLogiKd_sources += [
    'USB/API/libusb.cpp',
    'USB/API/libusb.hpp',
  ]
  config_h.set('HAVE_LIBUSB', 1)
endif

#! ----------

libudev_dep = dependency('libudev')

deps_versions_array += [
  ['LIBHIDAPI', hidapi_dep],
  ['LIBUSB', libusb_dep],
  ['LIBUDEV', libudev_dep],
]

#! ----------

foreach ident : [
  ['getpwnam',    '''#include <sys/types.h>
                     #include <pwd.h>'''],
  ['getgrnam',    '''#include <sys/types.h>
                     #include <grp.h>'''],
  ['initgroups',  '''#include <sys/types.h>
                     #include <grp.h>'''],
  ['setresgid',   '''#include <unistd.h>'''],
  ['setegid',     '''#include <unistd.h>'''],
  ['setregid',    '''#include <unistd.h>'''],
  ['setresuid',   '''#include <unistd.h>'''],
  ['seteuid',     '''#include <unistd.h>'''],
  ['setreuid',    '''#include <unistd.h>'''],
]
  have = cxx.has_function(ident[0], prefix: ident[1], args: '-D_GNU_SOURCE')
  if have
    config_h.set10('HAVE_' + ident[0].to_upper(), have)
  endif
endforeach

#! ----------

executable(
  'GLogiKd',
  GLogiKd_sources,
  include_directories : inc,
  dependencies : [
    thread_dep,
    boost_dep,
    boost_filesystem_dep,
    boost_program_options_dep,
    libudev_dep,
    libusb_dep,
    hidapi_dep,
    GKUtils_lib_dep,
    GKShared_lib_dep,
    GKDBus_lib_dep,
  ],
  install : true,
  install_dir : gk_sbindir,
  install_rpath : gk_libdir
)


