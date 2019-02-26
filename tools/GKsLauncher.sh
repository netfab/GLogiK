#!/bin/bash
# helper script, part of GLogiK project

if [ $# -lt 1 ]; then
	printf "this helper bash script requires at least one argument\n";
	exit 7;
fi

source GKDBus.sh || exit 5

# --

SESSION_SIGNAL_CMD="dbus-send --session --dest=${GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME} --type=signal"

function run_signal_cmd() {
	printf "running : ${SESSION_SIGNAL_CMD}\n"
	eval "${SESSION_SIGNAL_CMD}"
}

case "$1" in
	'--signal')
		case "$2" in
			'RestartRequest')
				SESSION_SIGNAL_CMD="${SESSION_SIGNAL_CMD} ${GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH}"
				SESSION_SIGNAL_CMD="${SESSION_SIGNAL_CMD} ${GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE}.$2"
			;;
			*)
				printf "sorry, missing or wrong argument $2\n"
				exit 8
			;;
		esac
		run_signal_cmd
	;;
	*)
		printf "wrong argument\n"
	;;
esac
