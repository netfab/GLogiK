#!/bin/bash

# Small bash script written as media keys support example
# Usage:
# ------
#		mediakey CMD
#
# list of known commands:
# -----------------------
#	pauseplay|stop|next|prev|togglemute|raisevolume|lowervolume
#
# list of currently suppported players:
# -------------------------------------
#	* mpd/mpc
#
#

# -- -- -- -- -- -- -- -- #

function check_binary() {
	type ${1} >/dev/null 2>&1 || \
		{ echo >&2 "I require ${1} but not in PATH. Aborting."; exit 1; }
}

function find_process_pid() {
	pgrep -u "${USER}" -x "${1}"
}

function set_process_status() {
	local myprocess="${1}"
	myprocess="${myprocess^^}"
	local cmd="export ${myprocess}_ON=0"
	eval "${cmd}"

	ppid=$(find_process_pid ${1})
	if [[ ! -z "${ppid}" ]]; then
		printf "found ${1} process. pid: ${ppid}\n"
		local cmd="export ${myprocess}_ON=1"
		eval "${cmd}"
	fi
}

# -- -- -- -- -- -- -- -- #

CMD=${1}

declare -r COMMANDS='pauseplay|stop|next|prev|togglemute|raisevolume|lowervolume'

declare -r OLDIFS="${IFS}"
IFS='|'
declare found=1
for x in ${COMMANDS}; do
	unset ${x}
	if [[ "${CMD}" == "${x}" ]]; then
		found=0
	fi
done
IFS="${OLDIFS}"
if [[ ${found} -ne 0 ]]; then
	printf "unknown command : ${CMD}\n"
	exit 7
fi

# -- -- -- -- -- -- -- -- #

function pauseplay() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc toggle
	fi
}

function stop() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc stop
	fi
}

function next() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc next
	fi
}

function prev() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc prev
	fi
}

function togglemute() {
	### https://wiki.archlinux.org/index.php/PulseAudio/Examples
	## list names
	# ------------
	# $ pacmd list-sources | grep -e device.string -e 'name:'
	#	name: <alsa_output.pci-0000_03_00.0.analog-stereo.monitor>
	#	device.string = "1"

	## toggle mute
	# ------------
	# $ pactl set-sink-mute alsa_output.pci-0000_03_00.0.analog-stereo toggle
	# $ pactl set-sink-mute 1 toggle

	# TODO could try to implement this by apps
	pactl set-sink-mute 1 toggle
}

function raisevolume() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc volume +5
	fi
}

function lowervolume() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		mpc volume -5
	fi
}

# -- -- -- -- -- -- -- -- #

set_process_status mpd

# TODO audacious support
if [[ ${MPD_ON} -eq 1 ]]; then
	${CMD} mpc
else
	printf "do not know which player to contact\n"
	exit 2
fi

exit $?

