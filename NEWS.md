
---

## version 0.8.x - 2019-xx-xx (unreleased)
#### Changes / Fixs
 * fix potential memory leaks when building Qt5 interface
 * remove media key events old implementation. Now simulate XF86 media
   events by linking desktop service to following X11 libraries :
	- x11-libs/libX11
	- x11-libs/libXtst

---

## version 0.8.6 - 2019-04-01
#### Added devices :
 * Logitech G510s Gaming Keyboard with onboard audio enabled (046d:c22e)

#### Changes / Fixs
 * several fixs regarding GLogiKs desktop service start
 * miscellaneous fixs

#### Added features :
 * added Qt5 Graphical User Interface :
	- ability to start desktop service if necessary
	- ability to control device state (start/stop/restart)
	- ability to modify device backlight color

---

## version 0.8.4 - 2019-02-26
#### Added features :
 * added desktop launcher binary
 * added GKDBus signals introspectability
 * ability to enable/disable LCD screen plugins from userland
 * check for version mismatch between daemon and clients

#### Changes / Fixs
 * run media commands synchronously
 * fix GKDBus signals implementation and usage
 * the desktop file now run the launcher binary (instead of GLogiKs)

---

## version 0.8.2 - 2018-09-10
#### Added features :
 * added LCD keys L1-L2-L3-L4-L5 support into LCD plugins

#### Changed
 * updated Splashscreen LCD plugin
 * updated SystemMonitor LCD plugin

#### Fixs
 * Fix wrong macro clearing behavior

---

## version 0.8.0 - 2018-08-09
#### Added features :
 * multimedia keys support
 * beginning of LCD screen support
 * now optionally depends on libnotify >= 0.7.7

#### Changed
 * now depends on dev-libs/boost >= 1.64.0 (boost process requirement)\
   https://github.com/klemens-morgenstern/boost-process

---

## version 0.6.0 - 2018-03-29 - (first release)
#### Added devices :
 * Logitech G510s Gaming Keyboard (046d:c22d)

#### Added features :
 * multi-users environment session switch support
 * hotplug device monitoring
 * device configuration file monitoring
 * record/assign/clear device macros
 * set device backlight color
 * control device via DBus (start/stop/restart/...)
 * handle multiple devices simultaneously

---

