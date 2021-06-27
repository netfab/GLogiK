GLogiK, daemon to handle special features on some gaming keyboards

Last release : version 0.8.12

Links
=====

 * [homepage / download](https://glogik.tuxfamily.org/)

Recognized devices
==================
 * Logitech G510s Gaming Keyboard (046d:c22d)
 * Logitech G510s Gaming Keyboard (046d:c22e) (onboard audio enabled)

Features
========

 * Macros keys (G-Keys) support (create, run or delete macros)
 * Keyboard backlight color support
 * Ability to set keyboard configuration on the fly in multi-users environment
 * Ability to handle multiple keyboard devices simultaneously
 * Hotplugged devices monitoring
 * Device status control via DBus
 * LCD screen support, including LCD keys
 * Multimedia keys support

Build Dependencies
==================

 * dev-libs/boost >= 1.64.0
 * a libudev provider :
   - sys-fs/eudev
   - sys-fs/udev
   - sys-apps/systemd
 * dev-libs/libevdev >= 1.5.7
 * dev-libs/libusb >= 1.0.19

Optional Build Dependencies
===========================

 * dev-libs/hidapi (libusb backend) >= 0.10.0
 * sys-apps/dbus >= 1.10.18
 * x11-libs/libICE
 * x11-libs/libSM
 * x11-libs/libX11
 * x11-libs/libXtst
 * Qt5 packages, including :
   - dev-qt/qtcore
   - dev-qt/qtgui
   - dev-qt/qtwidgets

Runtime Dependencies
====================

 * if built with D-Bus support, a user-seat-session manager is required to
 run desktop binaries :
   - sys-auth/elogind
   - sys-apps/systemd

GLogiK Daemon and Desktop Service
=================================

The GLogiKd daemon starts as root and drops its privileges.  
See following configure options :
 * --with-glogikd-user : defaults to glogikd
 * --with-glogikd-group : defaults to usb

For desktop users, see also :
 * --with-glogiks-group : defaults to glogiks

D-Bus support is required to build and run desktop binaries.

Users who wants to run the GLogiKs desktop service must be in the glogiks
group (or whatever is defined with the above option).

