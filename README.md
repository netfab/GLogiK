
# GLogiK, daemon to handle special features on gaming keyboards

Status : in development.
Recognized devices :
	- Logitech G510s Gaming Keyboard (046d:c22d)

## Build Dependencies :
	- >=dev-libs/boost-1.62.0
	- >=dev-libs/libusb-1.0.19
	- virtual/libudev
	- >=dev-libs/libevdev-1.5.7
	- >=sys-apps/dbus-1.10.18
	- >=x11-libs/libICE-1.0.9
	- >=x11-libs/libSM-1.2.2

## Runtime Dependencies :
	- >=sys-auth/consolekit-1.1.2 **or** systemd-logind

GLogiK is untested with previous versions of each dependency, but it may work.
libusb-0.1 is not supported. Feel free to report any (non-)working version.

## GLogiK Daemon and Desktop Service
The GLogiKd daemon starts as root and drops its privileges.
See following configure options :
	- --with-glogikd-user : defaults to glogikd
	- --with-glogikd-group : defaults to plugdev
For desktop users, see also :
	- --with-glogiks-group : defaults to glogiks
Users who wants to run the GLogiKs desktop service must be in the glogiks group.
(or whatever is defined with the above option).

