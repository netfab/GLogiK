#!/bin/bash
#
#	This file is part of GLogiK project.
#	GLogiK, daemon to handle special features on gaming keyboards
#	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
#
# Small bash script written as media keys support example
# Usage:
# ------
#		mediakey.sh <command>
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
		{ echo >&2 "I require ${1}, but not found in PATH. Aborting."; exit 1; }
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

function get_volume() {
	if [[ "${1}" = "mpc" ]] ; then
		shift
		local volume="$@"
		volume="${volume##*volume:}"
		volume="${volume%%\%*}"
		volume="${volume//[[:space:]]/}"
		printf "%s\n" "${volume}"
	fi
}

# -- -- -- -- -- -- -- -- #

declare -r CMD="${1}"

declare -r COMMANDS='pauseplay|stop|next|prev|togglemute|raisevolume|lowervolume'

if [[ -z "${CMD}" ]]; then
	printf "I require one command as parameter ; aborting\n"
	printf "Known commands : ${COMMANDS}\n"
	exit 8
fi

declare -r OLDIFS="${IFS}"
IFS='|'
declare found=1
for x in ${COMMANDS}; do
	unset ${x}
	if [[ "${CMD}" == "${x}" ]]; then
		found=0
		break
	fi
done
IFS="${OLDIFS}"
if [[ ${found} -ne 0 ]]; then
	printf "I require one command as parameter ; aborting\n"
	printf "Known commands : ${COMMANDS}\n"
	exit 7
fi
unset found

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

# make sure that the last line printed is the volume level
# used by devicesHandler for desktop notification
function raisevolume() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		local out=$(mpc volume +5)
		local volume=$(get_volume mpc "${out}")
		printf "%s\n" "${out}"
		printf "%s\n" "${volume}"
	fi
}

# make sure that the last line printed is the volume level
# used by devicesHandler for desktop notification
function lowervolume() {
	if [[ "${1}" = "mpc" ]] ; then
		check_binary mpc
		local out=$(mpc volume -5)
		local volume=$(get_volume mpc "${out}")
		printf "%s\n" "${out}"
		printf "%s\n" "${volume}"
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

