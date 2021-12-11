
GKUtils_sources = [
  'randomGenerator.cpp',
  'randomGenerator.hpp',
  'FSNotify.cpp',
  'FSNotify.hpp',
  'filesystem.cpp',
  'filesystem.hpp',
  'functions.cpp',
  'functions.hpp',
  'exception.cpp',
  'exception.hpp',
  'XDGUserDirs.cpp',
  'XDGUserDirs.hpp',
  'GKLogging.cpp',
  'GKLogging.hpp',
  'utils.hpp'
]

GKUtils_lib = library(
	'GKUtils',
	GKUtils_sources,
	include_directories : inc,
	dependencies : [
		boost_dep,
		boost_thread_dep,
		boost_log_setup_dep,
		boost_log_dep,
		boost_system_dep,
		boost_filesystem_dep
	],
	version : '0.0.0',
	soversion : '0',
	install : true,
)

GKUtils_lib_dep = declare_dependency(
  link_with : GKUtils_lib,
  dependencies : [
	boost_dep,
	boost_log_dep,
  ]
)
