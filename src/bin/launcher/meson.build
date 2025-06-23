

GKsLauncher_sources = [
  'main.cpp',
  'DBusHandler.cpp',
  'DBusHandler.hpp',
  'launcher.cpp',
  'launcher.hpp',
]

executable(
  'GKsLauncher',
  GKsLauncher_sources,
  include_directories : inc,
  dependencies : [
	thread_dep,
	boost_dep,
	boost_system_dep,
	boost_filesystem_dep,
	boost_process_dep,
	boost_program_options_dep,
	GKUtils_lib_dep,
	GKShared_lib_dep,
	GKDBus_lib_dep,
  ],
  install : true,
  install_rpath : gk_libdir
)

