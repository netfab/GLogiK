#!/bin/bash
# helper script, part of GLogiK project

if [ $# -lt 1 ]; then
	printf "this helper bash script requires at least one argument\n";
	exit 7;
fi

DAEMON_DBUS_ROOT_NODE_PATH="/com/glogik/Daemon"
# --
DAEMON_DEVICES_MANAGER_DBUS_OBJECT="DevicesManager"
DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH="${DAEMON_DBUS_ROOT_NODE_PATH}/${DAEMON_DEVICES_MANAGER_DBUS_OBJECT}"
DAEMON_DEVICES_MANAGER_DBUS_INTERFACE="com.glogik.Daemon.Device1"

# --
DAEMON_CLIENTS_MANAGER_DBUS_OBJECT="ClientsManager"
DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH="${DAEMON_DBUS_ROOT_NODE_PATH}/${DAEMON_CLIENTS_MANAGER_DBUS_OBJECT}"
DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE="com.glogik.Daemon.Client1"

DESKTOP_SERVICE_BUS_CONNECTION_NAME="com.glogik.Client"
# --

SYSTEM_SIGNAL_CMD="dbus-send --system --dest=${DESKTOP_SERVICE_BUS_CONNECTION_NAME} --type=signal"

METHOD_CMD="dbus-send --system --dest=${DESKTOP_SERVICE_BUS_CONNECTION_NAME} --type=method_call"
METHOD_CMD="${METHOD_CMD} --print-reply --reply-timeout=2000"

function run_signal_cmd() {
	printf "running : ${SYSTEM_SIGNAL_CMD}\n"
	eval "${SYSTEM_SIGNAL_CMD}"
}

function run_method_cmd() {
	printf "running : ${METHOD_CMD}\n"
	eval "${METHOD_CMD}"
}

case "$1" in
	'--signal')
		case "$2" in
			'ReportYourself' | 'DaemonIsStopping' | 'DaemonIsStarting')
				SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH}"
				SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE}.$2"
			;;
			# those signals are requiring parameters
			'DevicesStarted' | 'DevicesStopped' | 'DevicesUnplugged')
				SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH}"
				SYSTEM_SIGNAL_CMD="${SYSTEM_SIGNAL_CMD} ${DAEMON_DEVICES_MANAGER_DBUS_INTERFACE}.$2"
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
