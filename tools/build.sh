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
	printf " -A|--autotools\n"
	printf " -D|--distcheck (only with autotools)\n"
	printf " -M|--meson\n"
}

# -- -- -- -- -- -- -- -- -- #

declare -i autotools=0
declare -i distcheck=0
declare -i clean=0
declare -i debug=0
declare -i meson=0
declare CONF_FLAGS=""

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
		-A|--autotools)
			autotools=1
		;;
		-D|--distcheck)
			distcheck=1
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

function autotools_make_targets()
{
	local -i ret=0
	for x in ${@}; do
		printf "${BLDBLU}[${BLDGRN}running target${BLDBLU}]${TXTRST}: make ${x}\n"
		make ${x}
		ret=$?
		[[ ${ret} -ne 0 ]] && break;
	done
	return $ret
}

function autotools_clean()
{
	printf "${BLDRED}CLEANING WITH AUTOTOOLS${TXTRST}\n"
	rm -f -v .configured
	autotools_make_targets fullclean_it
}

function autotools_build()
{
	printf "${BLDRED}BUILDING WITH AUTOTOOLS${TXTRST}\n"
	if [[ ${debug} -eq 1 ]]; then
		export DEBUG_BUILD=1
	fi

	export EXTRA_FLAGS="${CONF_FLAGS} --enable-option-checking=fatal"

	if [[ ! -f .configured ]]; then
		autotools_make_targets autoreconf_it configure_it
		if [[ $? -eq 0 ]]; then
			touch .configured
		else
			die "AUTOTOOLS CONFIGURATION FAILURE"
		fi
	fi

	if [[ ${distcheck} -eq 1 ]]; then
		autotools_make_targets distcheck_it
	else
		autotools_make_targets build_it install_it
	fi

	local -r ret=$?
	return $ret
}

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

	local -r ret=$?

	if [[ ${distcheck} -eq 1 ]]; then
		warn "--distcheck not available with meson"
	fi

	return $ret
}

# -- -- -- -- -- -- -- -- -- #

if [[ ${autotools} -eq 1 ]]; then
	if [[ ${clean} -eq 1 ]]; then
		autotools_clean
	else
		autotools_build
	fi
fi

if [[ ${meson} -eq 1 ]]; then
	if [[ ${clean} -eq 1 ]]; then
		meson_clean
	else
		meson_build
	fi
fi


