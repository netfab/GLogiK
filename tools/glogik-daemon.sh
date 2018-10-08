#!/bin/bash
# helper script, part of GLogiK project

if [ $# -lt 1 ]; then
	printf "this helper bash script requires at least one argument\n";
	exit 7;
fi

DAEMON_DEVICES_DBUS_ROOT_NODE="/com/glogik/Daemon"
DAEMON_DEVICES_MANAGER_DBUS_OBJECT="DevicesManager"
DAEMON_CLIENTS_MANAGER_DBUS_OBJECT="ClientsManager"
# --
DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH="${DAEMON_DEVICES_DBUS_ROOT_NODE}/${DAEMON_DEVICES_MANAGER_DBUS_OBJECT}"
DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH="${DAEMON_DEVICES_DBUS_ROOT_NODE}/${DAEMON_CLIENTS_MANAGER_DBUS_OBJECT}"
DAEMON_DEVICES_MANAGER_DBUS_INTERFACE="com.glogik.Daemon.Device1"
# --
METHOD_CMD="dbus-send --system --dest=com.glogik.Daemon --type=method_call"
METHOD_CMD="${METHOD_CMD} --print-reply --reply-timeout=2000"

function run_cmd() {
	printf "running : ${METHOD_CMD}\n"
	eval "${METHOD_CMD}"
}

function check_arg() {
	if [ -z "$@" ]; then
		printf "sorry, missing argument\n"
		exit 8
	fi
}

declare -l clientID=""

function ask_user_id() {
	printf "please type your clientID : "
	read -n 23 clientID
	printf "\n"
}

case "$1" in
	'--stop-device')
		check_arg $2
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.StopDevice"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\" string:\""$2"\""
		run_cmd
	;;
	'--start-device')
		check_arg $2
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.StartDevice"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\" string:\""$2"\""
		run_cmd
	;;
	'--restart-device')
		check_arg $2
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.RestartDevice"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\" string:\""$2"\""
		run_cmd
	;;
	'--get-started-devices')
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.GetStartedDevices"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\""
		run_cmd
	;;
	'--get-stopped-devices')
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.GetStoppedDevices"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\""
		run_cmd
	;;
	'--get-device-properties')
		check_arg $2
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.GetDeviceProperties"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\" string:\""$2"\""
		run_cmd
	;;
	'--set-device-backlight-color')
		check_arg $5
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.SetDeviceBacklightColor"
		ask_user_id
		METHOD_CMD="${METHOD_CMD} string:\"${clientID}\" string:\""$2"\" byte:\""$3"\" byte:\""$4"\" byte:\""$5"\""
		run_cmd
	;;
	'--introspect')
		case "$2" in
			'DevicesManager')	OBJECT_PATH="${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}" ;;
			'ClientsManager')	OBJECT_PATH="${DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH}" ;;
			*)
				printf "missing or wrong argument $2\n"
				printf "intropecting root node ...\n"
				OBJECT_PATH="${DAEMON_DEVICES_DBUS_ROOT_NODE}"
			;;
		esac
		METHOD_CMD="${METHOD_CMD} ${OBJECT_PATH}"
		METHOD_CMD="${METHOD_CMD} org.freedesktop.DBus.Introspectable.Introspect"
		run_cmd
	;;
	*)
		printf "wrong argument\n"
	;;
esac

