
GKcQt_sources = [
  'AboutDialog/AboutDialogTab.cpp',
  'AboutDialog/AboutDialogTab.hpp',
  'AboutDialog/AboutTab.cpp',
  'AboutDialog/AboutTab.hpp',
  'AboutDialog/DependenciesTab.cpp',
  'AboutDialog/DependenciesTab.hpp',
  'AboutDialog/LicenseTab.cpp',
  'AboutDialog/LicenseTab.hpp',
  'AboutDialog/AboutDialog.cpp',
  'AboutDialog/AboutDialog.hpp',
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

qt_dep = null_dep

if enable_qt5
  qt_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])
endif

if enable_qt6
  qt_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])
endif

deps_versions_array += [
  ['QT', qt_dep],
]

executable(
  'GKcQt',
  GKcQt_sources,
  include_directories : inc,
  dependencies : [
	qt_dep,
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

