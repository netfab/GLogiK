#!/bin/bash

# -- -- -- -- -- -- -- -- -- #

export LANG=C

declare -r BLDRED='\e[1;31m' # Red Bold
declare -r BLDGRN='\e[1;32m' # Green Bold
declare -r BLDYLW='\e[1;33m' # Yellow Bold
declare -r BLDBLU='\e[1;34m' # Blue Bold
declare -r BLDPUR='\e[1;35m' # Purple
declare -r TXTRST='\e[0m'    # Text Reset

# -- -- -- -- -- -- -- -- -- #

function die()
{
	printf " ${BLDRED}ERROR${TXTRST}: ${@}\n" >&2
	exit 7
}

function warn()
{
	printf " ${BLDYLW}WARNING${TXTRST}: ${@}\n" >&2
	exit 8
}

function info()
{
	printf " ${BLDPUR}INFO: ${@}${TXTRST}\n" >&2
}

function check_ret()
{
	if [[ $1 -ne 0 ]]; then
		die "returned code: $1"
	fi
}





# --

bash build.sh -c -A -M

# --

info 'BUILD: DBus:off'
bash build.sh -A -M -d --disable-dbus --disable-qt6
check_ret $?
bash build.sh -c -A -M
check_ret $?
info 'CLEAN: DBus:off'

# --

info 'BUILD: debug:off'
bash build.sh -A -M --disable-debug
check_ret $?
bash build.sh -c -A -M
check_ret $?
info 'CLEAN: debug:off'

# --

info 'BUILD: hidapi:off QT5:on'
env WANTED_QT='QT5' bash build.sh -A -M -d --disable-hidapi --disable-qt6 --enable-qt5
check_ret $?
bash build.sh -c -A -M
check_ret $?
info 'CLEAN: hidapi:off QT5:on'

# --

info 'BUILD: debug: all on'
bash build.sh -A -M -d --enable-debug-gkdbus --enable-debug-keys \
						--enable-debug-lcd-plugins --enable-debug-libusb --enable-debug-pbmfont
check_ret $?
bash build.sh -c -A -M
check_ret $?
info 'CLEAN: debug: all on'

# --

