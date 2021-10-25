project(
  'GLogiK',
  'cpp',
  default_options: [
	'cpp_std=c++14',
	'debug=false'
  ],
  license: 'GPL-3',
  version: '0.8.13-git'
)

#! ----------

gk_prefix = get_option('prefix')
gk_sysconfdir = join_paths(gk_prefix, get_option('sysconfdir'))
gk_libdir = join_paths(gk_prefix, get_option('libdir'))
gk_sbindir = join_paths(gk_prefix, get_option('sbindir'))
gk_bindir = join_paths(gk_prefix, get_option('bindir'))
gk_datadir = join_paths(gk_prefix, get_option('datadir'))

pbm_data_dir = gk_datadir / meson.project_name() / 'pbm'

#! ----------

thread_dep = dependency('threads')

#! boost deps
#! ----------
boost_static = false
boost_minimal_version = '>=1.64.0'
boost_dependency_name = 'boost'

boost_dep = dependency(
  boost_dependency_name,
  version : boost_minimal_version,
  static : boost_static
)

boost_system_dep = dependency(
  boost_dependency_name,
  modules : ['system'],
  static : boost_static
)

boost_filesystem_dep = dependency(
  boost_dependency_name,
  modules : ['filesystem'],
  static : boost_static
)

boost_thread_dep = dependency(
  boost_dependency_name,
  modules : ['thread'],
  static : boost_static
)

boost_log_setup_dep = dependency(
  boost_dependency_name,
  modules : ['log_setup'],
  static : boost_static
)

boost_log_dep = dependency(
  boost_dependency_name,
  modules : ['log'],
  static : boost_static
)

boost_program_options_dep = dependency(
  boost_dependency_name,
  modules : ['program_options'],
  static : boost_static
)

boost_serialization_dep = dependency(
  boost_dependency_name,
  modules : ['serialization'],
  static : boost_static
)

boost_date_time_dep = dependency(
  boost_dependency_name,
  modules : ['date_time'],
  static : boost_static
)

#! ----------

enable_debug = get_option('debug')

enable_debug_keys = false
enable_debug_gkdbus = false
enable_debug_libusb = false
enable_debug_pbmfont = false
enable_debug_lcd_plugins = false

if enable_debug
  enable_debug_gkdbus = get_option('debug_gkdbus')
  enable_debug_keys = get_option('debug_keys')
  enable_debug_libusb = get_option('debug_libusb')
  enable_debug_pbmfont = get_option('debug_pbmfont')
  enable_debug_lcd_plugins = get_option('debug_lcd_plugins')
endif

glogikd_user = get_option('daemon_user')
glogikd_group = get_option('daemon_group')
glogiks_group = get_option('desktop_service_group')

#! ----------

debug_dir = '/var/tmp/GKDebug'

define_array = [
  ['PACKAGE', meson.project_name()],
  ['PACKAGE_NAME', meson.project_name()],
  ['PACKAGE_URL', 'https://glogik.tuxfamily.org/'],
  ['PACKAGE_VERSION', meson.project_version()],
  ['VERSION', meson.project_version()],
  ['DEBUG_DIR', debug_dir],
  ['GLOGIKD_USER', glogikd_user],
  ['GLOGIKD_GROUP', glogikd_group],
  ['DATA_DIR', gk_datadir],
  ['PBM_DATA_DIR', pbm_data_dir],
]

define_dep_array = [
  ['HAVE_BOOST_SYSTEM', boost_system_dep.found()],
  ['HAVE_BOOST_FILESYSTEM', boost_filesystem_dep.found()],
  ['HAVE_BOOST_THREAD', boost_thread_dep.found()],
  ['HAVE_BOOST_LOG_SETUP', boost_log_setup_dep.found()],
  ['HAVE_BOOST_LOG', boost_log_dep.found()],
  ['HAVE_BOOST_PROGRAM_OPTIONS', boost_program_options_dep.found()],
  ['HAVE_BOOST_SERIALIZATION', boost_serialization_dep.found()],
  ['HAVE_BOOST_DATE_TIME', boost_date_time_dep.found()],
  ['DEBUGGING_ON', enable_debug],
  ['DEBUG_KEYS', enable_debug_keys],
  ['DEBUG_GKDBUS_SUBOBJECTS', enable_debug_gkdbus],
  ['DEBUG_LIBUSB_EXTRA', enable_debug_libusb],
  ['DEBUG_PBMFONT', enable_debug_pbmfont],
  ['DEBUG_LCD_PLUGINS', enable_debug_lcd_plugins],
]

config_h = configuration_data()

foreach define: define_array
  config_h.set_quoted(define[0], define[1])
endforeach

foreach define: define_dep_array
  config_h.set10(define[0], define[1])
endforeach

#! ----------

inc = include_directories('.', 'src')

enable_dbus = get_option('dbus')
enable_hidapi = get_option('hidapi')
enable_qt5 = get_option('qt5')

#! ----------
null_dep = dependency('', required: false)
dbus_dep = null_dep
GKDBus_lib_dep = null_dep

#! ----------
cdata = configuration_data()
#! used by data/dbus
cdata.set('GLOGIKD_USER', glogikd_user)
cdata.set('GLOGIKS_GROUP', glogiks_group)
#! used by data/init
cdata.set('INIT_NEED_DBUS', 'dbus')
#! used by data/udev
cdata.set('DEVICES_MODE', '0660')
cdata.set('GLOGIKD_GROUP', glogikd_group)
#! used by data/tmpfiles.d
cdata.set('DEBUG_DIR', debug_dir)

#! ----------
subdir('src/lib/utils')  #! GKUtils_lib_dep
subdir('src/lib/shared') #! GKShared_lib_dep

if enable_dbus
  dbus_dep = dependency('dbus-1', version: '>=1.10.18')
  subdir('src/lib/dbus') #! GKDBus_lib_dep
  subdir('data/dbus')
endif

subdir('src/bin/daemon')
subdir('data/init')
subdir('data/udev')

subdir('data/pbm')

#! ----------

if enable_dbus
  subdir('src/bin/service')
  subdir('src/bin/launcher')
  subdir('data/autostart')

  if enable_qt5
	subdir('src/bin/gui/Qt')
	subdir('data/icons')
	subdir('data/applications')
  endif
endif

if enable_debug
	subdir('data/tmpfiles.d')
endif

config_h.set10('GKDBUS', dbus_dep.found())

#! ----------

configure_file(
  output : 'config.h',
  configuration : config_h
)

#! ----------

report = '\nPaths:\n'
report += '======\n'
report += '       prefix: ' + gk_prefix + '\n'
report += '   sysconfdir: ' + gk_sysconfdir + '\n'
report += '       libdir: ' + gk_libdir + '\n'
report += '       bindir: ' + gk_bindir + '\n'
report += '      sbindir: ' + gk_sbindir + '\n'
report += '      datadir: ' + gk_datadir + '\n'
report += ' udevrulesdir: ' + udev_rules_dir + '\n'
report += '\nBuild options:\n'
report += '==============\n'
report += '   dbus: ' + enable_dbus.to_string() + '\n'
report += ' hidapi: ' + enable_hidapi.to_string() + '\n'
report += '    qt5: ' + enable_qt5.to_string() + '\n'
report += '\nDebug options:\n'
report += '==============\n'
report += '             debug: ' + enable_debug.to_string() + '\n'
report += '        debug keys: ' + enable_debug_keys.to_string() + '\n'
report += '      debug gkdbus: ' + enable_debug_gkdbus.to_string() + '\n'
report += '      debug libusb: ' + enable_debug_libusb.to_string() + '\n'
report += '     debug pbmfont: ' + enable_debug_pbmfont.to_string() + '\n'
report += ' debug LCD plugins: ' + enable_debug_lcd_plugins.to_string() + '\n'
report += '\nConfiguration:\n'
report += '==============\n'
report += '           daemon user: ' + glogikd_user + '\n'
report += '          daemon group: ' + glogikd_group + '\n'
report += ' desktop service group: ' + glogiks_group + '\n'

message(report)

