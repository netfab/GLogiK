#!/bin/bash
# helper script, part of GLogiK project

if [ $# -ne 2 ]; then
	printf "this helper bash script requires exactly two arguments\n";
	exit 7;
fi

DESKTOP_SERVICE_DBUS_ROOT_NODE="/com/glogik/Desktop/Service"
# --
DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT="SystemMessageHandler"
DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH="${DESKTOP_SERVICE_DBUS_ROOT_NODE}/${DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT}"
DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE="com.glogik.Desktop.Service.SystemMessageHandler1"
# --

SYSTEM_SIGNAL_CMD="dbus-send --system --dest=com.glogik.Desktop.Service --type=signal"

function run_cmd() {
	printf "running : ${SYSTEM_SIGNAL_CMD}\n"
	eval "${SYSTEM_SIGNAL_CMD}"
}

case "$1" in
	'--signal')
		case "$2" in
			'SomethingChanged')
			;;
			*)
				printf "sorry, missing or wrong argument $2\n"
				exit 8
			;;
		esac
		SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_OBJECT_PATH}"
		SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DESKTOP_SERVICE_SYSTEM_MESSAGE_HANDLER_DBUS_INTERFACE}.$2"
		run_cmd
	;;
	*)
		printf "wrong argument\n"
	;;
esac
