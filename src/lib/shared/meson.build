
GKShared_sources = [
  'deviceProperties.cpp',
  'deviceProperties.hpp',
  'deviceConfigurationFile.cpp',
  'deviceConfigurationFile.hpp',
  'GKeysBanksCapability.cpp',
  'GKeysBanksCapability.hpp',
  'GKeysMacro.cpp',
  'GKeysMacro.hpp',
  'glogik.cpp',
  'glogik.hpp',
]

ice_dep = null_dep
sm_dep = null_dep

if enable_dbus
  ice_dep = dependency('ice')
  sm_dep = dependency('sm')

  GKShared_sources += [
	'sessionManager.cpp',
	'sessionManager.hpp',
  ]
endif

deps_versions_array += [
  ['ICE', ice_dep],
  ['SM', sm_dep],
]

GKShared_lib = library(
	'GKShared',
	GKShared_sources,
	include_directories : inc,
	dependencies : [
		boost_dep,
		boost_serialization_dep,
		ice_dep,
		sm_dep,
		GKUtils_lib_dep,
	],
	version : '0.0.0',
	soversion : '0',
	install : true,
)


GKShared_lib_dep = declare_dependency(
	link_with: GKShared_lib
)
