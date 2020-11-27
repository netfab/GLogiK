
0.8.xx (2020-xx-xx)
===================
- Improved LCDPlugin progress bar drawing function
- Internal work on GKDBus allowing binaries to disconnect from
  busses and free allocated resources in the correct order
- Make D-Bus dependency optional. D-Bus dependency optional means that
  without D-bus support, only the daemon will be built. If you want
  desktop service and GUI support, you want D-Bus enabled
- Under-the-hood work to reorganize keyboard driver classes, allowing us
  to select USB library provider at configure time
- Added dev-libs/hidapi support (with libusb backend) and make it
  default over dev-libs/libusb
- Removed consolekit support
- Switch from plugdev group to usb group for devices access

0.8.10 (2020-08-02)
===================
- Implemented LCD plugin locking mechanism. Press L2 LCD key to
  lock/unlock current LCD plugin
- Fixed potential daemon crash when handling unresponsive clients
- Setted logind as default session tracker (instead of consolekit)
- Use /proc/meminfo pseudo file to get more precise value
  of current memory usage for systemMonitor LCD plugin
- Updated splashscreen LCD plugin
- Updated default DEBUG_DIR. Debug files are now created in a more
  secured way
- Miscellaneous fixs

0.8.8 (2019-05-12)
==================
- Added LCD screen plugins tab in Qt5 GUI. Implemented all related
  glue code into service and daemon
- Removed media key events old implementation. Now simulate XF86 media
  events by linking desktop service to following X11 libraries :
    * x11-libs/libX11
    * x11-libs/libXtst
- Fixed potential memory leaks when building Qt5 interface
- Fixed start request sent to launcher when Qt5 GUI is starting and
   desktop service is not running
- Miscellaneous fixs (launcher linking error, crash in GKDBus)

0.8.6 (2019-04-01)
==================
- Added device : Logitech G510s Gaming Keyboard (046d:c22e)
    (onboard audio enabled)
- Several fixs regarding GLogiKs desktop service start
- Miscellaneous fixs
- Added Qt5 Graphical User Interface :
    * Ability to start desktop service if necessary
	* Ability to control device state (start/stop/restart)
	* Ability to modify device backlight color

0.8.4 (2019-02-26)
==================
- Added desktop launcher binary
- Added GKDBus signals introspectability
- Ability to enable/disable LCD screen plugins from userland
- Check for version mismatch between daemon and clients
- Run media commands synchronously
- Fix GKDBus signals implementation and usage
- Desktop file now run the launcher binary (instead of GLogiKs)

0.8.2 (2018-09-10)
==================
- Added LCD keys L1-L2-L3-L4-L5 support into LCD plugins
- Updated Splashscreen LCD plugin
- Updated SystemMonitor LCD plugin
- Fix wrong macro clearing behavior

0.8.0 (2018-08-09)
==================
- Multimedia keys support
- Beginning of LCD screen support
- Optionally depends on libnotify >= 0.7.7
- now depends on dev-libs/boost >= 1.64.0 (boost process requirement)
    https://github.com/klemens-morgenstern/boost-process

0.6.0 (2018-03-29)
==================
- Added device : Logitech G510s Gaming Keyboard (046d:c22d)
- Multi-users environment session switch support
- Hotplug device monitoring
- Device configuration file monitoring
- Record/assign/clear device macros
- Set device backlight color
- Control device via DBus (start/stop/restart/...)
- Handle multiple devices simultaneously

