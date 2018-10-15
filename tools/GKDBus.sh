#!/bin/bash

# /* devices thread */
declare -r GLOGIK_DEVICE_THREAD_DBUS_BUS_CONNECTION_NAME="com.glogik.Device"

# /* daemon thread */
declare -r GLOGIK_DAEMON_DBUS_ROOT_NODE="Daemon"
declare -r GLOGIK_DAEMON_DBUS_ROOT_NODE_PATH="/com/glogik/Daemon"
declare -r GLOGIK_DAEMON_DBUS_BUS_CONNECTION_NAME="com.glogik.Daemon"
#	/* -- */
declare -r GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT="ClientsManager"
declare -r GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_OBJECT_PATH="/com/glogik/Daemon/ClientsManager"
declare -r GLOGIK_DAEMON_CLIENTS_MANAGER_DBUS_INTERFACE="com.glogik.Daemon.Client1"
#	/* -- */
declare -r GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT="DevicesManager"
declare -r GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_OBJECT_PATH="/com/glogik/Daemon/DevicesManager"
declare -r GLOGIK_DAEMON_DEVICES_MANAGER_DBUS_INTERFACE="com.glogik.Daemon.Device1"

# /* desktop service launcher */
declare -r GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE="Launcher"
declare -r GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_ROOT_NODE_PATH="/com/glogik/Launcher"
declare -r GLOGIK_DESKTOP_SERVICE_LAUNCHER_DBUS_BUS_CONNECTION_NAME="com.glogik.Launcher"

# /* desktop service */
declare -r GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE="Client"
declare -r GLOGIK_DESKTOP_SERVICE_DBUS_ROOT_NODE_PATH="/com/glogik/Client"
declare -r GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME="com.glogik.Client"
#	/* -- */
declare -r GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT="SessionMessageHandler"
declare -r GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH="/com/glogik/Client/SessionMessageHandler"
declare -r GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE="com.glogik.Client.SessionMessageHandler1"

