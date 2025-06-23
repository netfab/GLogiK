
GLogiKs_sources = [
  'main.cpp',
  'devicesHandler.cpp',
  'devicesHandler.hpp',
  'GKeysEventManager.cpp',
  'GKeysEventManager.hpp',
  'initLog.cpp',
  'initLog.hpp',
  'DBus.cpp',
  'DBus.hpp',
  'DBusHandler.cpp',
  'DBusHandler.hpp',
  'service.cpp',
  'service.hpp',
  'virtualKeyboard.cpp',
  'virtualKeyboard.hpp',
]

if enable_notifications
  GLogiKs_sources += [
	'desktopNotification.cpp',
	'desktopNotification.hpp',
  ]
endif

libevdev_dep = dependency('libevdev')
x11_dep = dependency('x11')
xtst_dep = dependency('xtst')

deps_versions_array += [
  ['LIBEVDEV', libevdev_dep],
  ['LIBX11', x11_dep],
  ['LIBXTST', xtst_dep],
]

executable(
  'GLogiKs',
  GLogiKs_sources,
  include_directories : inc,
  dependencies : [
	thread_dep,
	boost_dep,
	boost_system_dep,
	boost_filesystem_dep,
	boost_process_dep,
	boost_program_options_dep,
	boost_serialization_dep,
    libevdev_dep,
    libnotify_dep,
	x11_dep,
	xtst_dep,
	GKUtils_lib_dep,
	GKShared_lib_dep,
	GKDBus_lib_dep,
  ],
  install : true,
  install_rpath : gk_libdir
)

