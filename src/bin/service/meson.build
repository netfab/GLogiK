
GLogiKs_sources = [
  'main.cpp',
  'devicesHandler.cpp',
  'devicesHandler.hpp',
  'DBusHandler.cpp',
  'DBusHandler.hpp',
  'service.cpp',
  'service.hpp',
]

x11_dep = dependency('x11')
xtst_dep = dependency('xtst')

executable(
  'GLogiKs',
  GLogiKs_sources,
  include_directories : inc,
  dependencies : [
	thread_dep,
	boost_dep,
	boost_system_dep,
	boost_filesystem_dep,
	boost_program_options_dep,
	boost_serialization_dep,
	x11_dep,
	xtst_dep,
	GKUtils_lib_dep,
	GKShared_lib_dep,
	GKDBus_lib_dep,
  ],
  install : true,
  install_rpath : gk_libdir
)

