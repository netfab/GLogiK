#!/bin/bash

# -- -- -- -- -- -- -- -- -- #

export LANG=C

declare -r BLDRED='\e[1;31m' # Red Bold
declare -r BLDGRN='\e[1;32m' # Green Bold
declare -r BLDBLU='\e[1;34m' # Blue Bold
declare -r TXTRST='\e[0m'    # Text Reset

# relative paths
declare -r REL_BUILD_DIR='builddir'
declare -r REL_INSTALL_DIR='installdir'

# absolute paths
declare -r ABS_PREFIX_DEV_DIR='/var/tmp/devcxx'
declare -r BUILDSYSTEM_WORK_DIR="${ABS_PREFIX_DEV_DIR}/meson"
declare -r PACKAGE_WORK_DIR="${BUILDSYSTEM_WORK_DIR}/GLogiK"
declare -r ABS_BUILD_DIR="${PACKAGE_WORK_DIR}/${REL_BUILD_DIR}"
declare -r ABS_SOURCE_DIR="${PWD%\/*}"
declare -r ABS_INSTALL_DIR="${PACKAGE_WORK_DIR}/${REL_INSTALL_DIR}"

# ccache automatically enabled if found
# https://mesonbuild.com/Feature-autodetection.html#ccache
#export PATH="/usr/lib/ccache/bin:${PATH}"
export CCACHE_DIR='/var/tmp/devccache'

# -- -- -- -- -- -- -- -- -- #

function die() {
	printf " ${BLDRED}ERROR${TXTRST}: ${@}\n" >&2
	exit 7
}

function change_directory() {
	if [[ -d "${1}" ]]; then
		cd "${1}" || die "cd failure: ${1}"
	else
		die "directory ${1} does not exist"
	fi
}

function print_help() {
	printf " Known parameters:\n"
	printf " -C|--configure\n"
	printf " -c|--compile\n"
	printf " -i|--install\n"
	printf " -f|--fullclean\n"
}

# -- -- -- -- -- -- -- -- -- #

declare configure=0
declare compile=0
declare fullclean=0
declare install=0
declare stripbuild=0

declare options=''

function add_meson_option() {
	local -r option="${1##*-}"
	[[ ${#option} -eq 0 ]] && die "wrong parameter: ${1}"
	options+="-D${option}=${2} "
}

# default options values, overridable by command line parameters
add_meson_option 'dbus' 'true'
add_meson_option 'notifications' 'true'
add_meson_option 'libnotify' 'true'
#add_meson_option 'qt5' 'true'
add_meson_option 'qt6' 'true'

# parsing command line parameters
while [[ "${#}" -gt 0 ]]; do
	case "${1}" in
		-C|--configure)
			configure=1
		;;
		-c|--compile)
			configure=1
			compile=1
		;;
		-f|--fullclean)
			fullclean=1
		;;
		-h|--help)
			print_help
			exit 0
		;;
		-i|--install)
			configure=1
			compile=1
			install=1
		;;
		-s|--strip)
			stripbuild=1
		;;
		--enable-*)
			add_meson_option "${1}" 'true'
		;;
		--disable-*)
			add_meson_option "${1}" 'false'
		;;
		*)
			print_help
			die "unknown parameter: ${1}"
		;;
	esac
	shift
done

# -- -- -- -- -- -- -- -- -- #

function clean_action() {
	printf "${BLDRED}CLEANING WITH MESON${TXTRST}\n"

	function safe_remove_dir() {
		printf "removing « ${1} » directory ... "
		if [[ -d "${1}" ]]; then
			rm -rf "${1}" &> /dev/null
			if [[ $? -eq 0 ]]; then
				printf "${BLDBLU}[${BLDGRN}ok${BLDBLU}]${TXTRST}\n"
			else
				printf "${BLDBLU}[${BLDRED}ko${BLDBLU}]${TXTRST}\n"
			fi
		else
			printf "${BLDBLU}[${BLDRED}not found${BLDBLU}]${TXTRST}\n"
		fi
	}

	if [[ -d "${PACKAGE_WORK_DIR}" ]]; then
		change_directory "${PACKAGE_WORK_DIR}"

		safe_remove_dir "${REL_BUILD_DIR}"
		safe_remove_dir "${REL_INSTALL_DIR}"
	fi
}

function configure_action() {
	printf "${BLDRED}CONFIGURING WITH MESON${TXTRST}\n"

	# https://mesonbuild.com/Builtin-options.html#build-type-options
	local mesonconfigure=(
		meson setup
		-Doptimization=2
		-Dcpp_args="-pipe -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -Wformat=2 -Wfatal-errors"
		-Dwerror=true
		-Db_asneeded=true
		-Drelative_udev_rules_dir=true
		--prefix="${ABS_INSTALL_DIR}"
		-Ddocdir=GLogiK-9999
		# XXX with DESTDIR
		#--prefix=/usr
		#--datadir=/usr/share
		#--sysconfdir=/etc
		#--libdir=/usr/lib64
		"${ABS_BUILD_DIR}"
		"${ABS_SOURCE_DIR}"
	)

	if [[ ${stripbuild} -eq 1 ]]; then
		mesonconfigure+=('-Dstrip=true')
	fi

	if [[ ${#options} -ne 0 ]]; then
		mesonconfigure+=(
			${options}
		)
	fi

	(
		echo "${mesonconfigure[@]}" >&2
		"${mesonconfigure[@]}"
	) || die "configuration failure"

}

function compile_action() {
	printf "${BLDRED}BUILDING WITH MESON${TXTRST}\n"

	change_directory "${ABS_BUILD_DIR}"
	meson compile --verbose || die "compile failure"
}

function install_action() {
	printf "${BLDRED}INSTALLING WITH MESON${TXTRST}\n"

	change_directory "${ABS_BUILD_DIR}"
	#DESTDIR="${ABS_INSTALL_DIR}-DESTDIR" meson install || die "install failure"
	meson install || die "install failure"
}

# -- -- -- -- -- -- -- -- -- #

if [[ ${fullclean} -eq 1 ]]; then
	clean_action
fi

if [[ ${configure} -eq 1 ]]; then
	configure_action
fi

if [[ ${compile} -eq 1 ]]; then
	compile_action
fi

if [[ ${install} -eq 1 ]]; then
	install_action
fi

# -- -- -- -- -- -- -- -- -- #

