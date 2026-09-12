
GLogiKd_sources = [
  'main.cpp',
  'daemon.cpp',
  'daemon.hpp',
  'daemonControl.cpp',
  'daemonControl.hpp',
  'devicesManager.cpp',
  'devicesManager.hpp',
  'LCDScreenPluginsManager.cpp',
  'LCDScreenPluginsManager.hpp',
]

GLogiKd_sources += [
  'keyboards/logitech/G510/G510.hpp',
  'keyboards/logitech/G510/G510Base.cpp',
  'keyboards/logitech/G510/G510Base.hpp',
  'keyboards/logitech/G510/G510Detail.hpp',
  'keyboards/USBAPI/USBAPIDetail.hpp',
  'keyboards/USBAPI/USBDevice.cpp',
  'keyboards/USBAPI/USBDevice.hpp',
  'keyboards/USBAPI/USBDeviceID.cpp',
  'keyboards/USBAPI/USBDeviceID.hpp',
  'keyboards/USBAPI/usbinit.cpp',
  'keyboards/USBAPI/usbinit.hpp',
  'keyboards/KeyboardDriver.cpp',
  'keyboards/KeyboardDriver.hpp',
  'keyboards/KeyboardDriverDetail.hpp',
  'keyboards/USBKeyboardDriver.hpp',
]

GLogiKd_sources += [
  'LCDPlugins/PBM.hpp',
  'LCDPlugins/PBMFile.cpp',
  'LCDPlugins/PBMFile.hpp',
  'LCDPlugins/PBMFont.cpp',
  'LCDPlugins/PBMFont.hpp',
  'LCDPlugins/fonts.hpp',
  'LCDPlugins/fontsManager.cpp',
  'LCDPlugins/fontsManager.hpp',
  'LCDPlugins/LCDPlugin.cpp',
  'LCDPlugins/LCDPlugin.hpp',
  'LCDPlugins/splashscreen.cpp',
  'LCDPlugins/splashscreen.hpp',
  'LCDPlugins/cpu-stats/CPUData.cpp',
  'LCDPlugins/cpu-stats/CPUData.h',
  'LCDPlugins/cpu-stats/CPUSnapshot.cpp',
  'LCDPlugins/cpu-stats/CPUSnapshot.h',
  'LCDPlugins/netsnap/netSnapshots.cpp',
  'LCDPlugins/netsnap/netSnapshots.hpp',
  'LCDPlugins/systemMonitor.cpp',
  'LCDPlugins/systemMonitor.hpp',
  'LCDPlugins/coretemp.cpp',
  'LCDPlugins/coretemp.hpp',
  'LCDPlugins/endscreen.cpp',
  'LCDPlugins/endscreen.hpp',
]

if enable_dbus
  GLogiKd_sources += [
    'clientsManager.cpp',
    'clientsManager.hpp',
    'clientsSignals.cpp',
    'clientsSignals.hpp',
    'client.cpp',
    'client.hpp',
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
    'keyboards/USBAPI/hidapi.cpp',
    'keyboards/USBAPI/hidapi.hpp',
  ]
  config_h.set('HAVE_HIDAPI', 1)
elif libusb_dep.found()
  GLogiKd_sources += [
    'keyboards/USBAPI/libusb.cpp',
    'keyboards/USBAPI/libusb.hpp',
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


