
GKcQt5_sources = [
  'AboutDialog.cpp',
  'AboutDialog.hpp',
  'Tab.cpp',
  'Tab.hpp',
  'DaemonAndServiceTab.cpp',
  'DaemonAndServiceTab.hpp',
  'DeviceControlTab.cpp',
  'DeviceControlTab.hpp',
  'BacklightColorTab.cpp',
  'BacklightColorTab.hpp',
  'LCDPluginsTab.cpp',
  'LCDPluginsTab.hpp',
  'GKeysTab.cpp',
  'GKeysTab.hpp',
  'mainWindow.cpp',
  'mainWindow.hpp',
  'main.cpp',
]

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])

deps_versions_array += [
  ['QT5', qt5_dep],
]

executable(
  'GKcQt5',
  GKcQt5_sources,
  include_directories : inc,
  dependencies : [
	qt5_dep,
	boost_system_dep,
	boost_filesystem_dep,
	boost_program_options_dep,
	GKUtils_lib_dep,
	GKShared_lib_dep,
	GKDBus_lib_dep,
  ],
  install : true,
  install_rpath : gk_libdir
)

