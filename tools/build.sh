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

function die() {
	printf " ${BLDRED}ERROR${TXTRST}: ${@}\n" >&2
	exit 7
}

function warn() {
	printf " ${BLDYLW}WARNING${TXTRST}: ${@}\n" >&2
	exit 8
}

function print_help() {
	printf " Known parameters:\n"
	printf " -c|--clean\n"
	printf " -d|--debug\n"
	printf " -h|--help\n"
	printf " -M|--meson\n"
}

# -- -- -- -- -- -- -- -- -- #

declare -i clean=0
declare -i debug=0
declare -i meson=0
declare CONF_FLAGS=""

# force meson by default
meson=1

# parsing command line parameters
while [[ "${#}" -gt 0 ]]; do
	case "${1}" in
		-c|--clean)
			clean=1
		;;
		-d|--debug)
			debug=1
		;;
		-h|--help)
			print_help
			exit 0
		;;
		-M|--meson)
			meson=1
		;;
		# FIXME standardize and support other options
		# (daemon-user, daemon-group, desktop-service-group, ...)
		--enable-*)
			CONF_FLAGS+=" ${1}"
		;;
		--disable-*)
			CONF_FLAGS+=" ${1}"
		;;
		*)
			print_help
			die "unknown parameter: ${1}"
		;;
	esac
	shift
done

# -- -- -- -- -- -- -- -- -- #

function meson_clean()
{
	bash meson.sh --fullclean
}

function meson_build()
{
	local debug_opt=
	if [[ ${debug} -eq 1 ]]; then
		debug_opt='--enable-debug'
	fi

	bash meson.sh --install ${debug_opt} ${CONF_FLAGS}

	return $?
}

# -- -- -- -- -- -- -- -- -- #

if [[ ${meson} -eq 1 ]]; then
	if [[ ${clean} -eq 1 ]]; then
		meson_clean
	else
		meson_build
	fi
fi


