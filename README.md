### GLogiK, daemon to handle special features on some gaming keyboards

Last release : version 0.8.10

### Links
 - [homepage / download](https://glogik.tuxfamily.org/)

### Recognized devices
 - Logitech G510s Gaming Keyboard (046d:c22d)
 - Logitech G510s Gaming Keyboard (046d:c22e) (onboard audio enabled)

### Features
 - Macros keys (G-Keys) support (create, run or delete macros)
 - Keyboard backlight color support
 - Monitoring text based devices configuration files
 - Ability to set keyboard configuration on the fly in multi-users environment
 - Ability to handle multiple keyboard devices simultaneously
 - Hotplugged devices monitoring
 - Device status control via DBus
 - LCD screen support, including LCD keys
 - Multimedia keys support

### Build Dependencies :
 - dev-libs/boost 1.64.0 or later
 - dev-libs/libusb 1.0.19 or later
 - virtual/libudev
 - dev-libs/libevdev 1.5.7 or later
 - sys-apps/dbus 1.10.18 or later
 - x11-libs/libICE 1.0.9 or later
 - x11-libs/libSM 1.2.2 or later
 - x11-libs/libX11
 - x11-libs/libXtst

### Optional Build Dependencies
 - Qt5 packages, including :
    * dev-qt/qtcore
    * dev-qt/qtgui
    * dev-qt/qtwidgets


### Runtime Dependencies :
 - sys-auth/consolekit 1.1.2 or later, or systemd-logind

GLogiK is untested with previous versions of each dependency, but it may work.  
libusb-0.1 is not supported. Feel free to report any (non-)working version.

### GLogiK Daemon and Desktop Service
The GLogiKd daemon starts as root and drops its privileges.  
See following configure options :
 - --with-glogikd-user : defaults to glogikd
 - --with-glogikd-group : defaults to plugdev

For desktop users, see also :
 - --with-glogiks-group : defaults to glogiks

Users who wants to run the GLogiKs desktop service must be in the glogiks group.
(or whatever is defined with the above option).

