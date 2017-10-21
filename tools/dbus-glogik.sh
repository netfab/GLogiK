#!/bin/bash

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
	printf "this helper bash script requires at least one and at most two arguments\n";
	exit 7;
fi

case $1 in
	'--send-clients-somethingchanged')
		dbus-send --system --dest=com.glogik.Desktop.Service --type=signal \
			/com/glogik/Desktop/Service/SystemMessageHandler \
			com.glogik.Desktop.Service.SystemMessageHandler1.SomethingChanged
	;;
	'--send-daemon-stop-device')
		dbus-send --system --dest=com.glogik.Daemon --type=method_call \
		--print-reply --reply-timeout=2000 /com/glogik/Daemon/DevicesManager \
		com.glogik.Daemon.Device1.Stop string:"$2"
	;;
esac

