#!/bin/bash

EXPECTED_VERSION="$@"
if [ -z "${EXPECTED_VERSION}" ]; then
	printf "error version expected"
	exit 7
fi

function do_release_dir() {
	local PREFIX_DEV_DIR="/var/tmp/devcxx"
	local build_dir="${PREFIX_DEV_DIR}/build"

	
	local release_file="${build_dir}/GLogiK-${EXPECTED_VERSION}.tar.gz"
	local release_dir="${PREFIX_DEV_DIR}/GLogiK-${EXPECTED_VERSION}"
	if [ -f "${release_file}" ]; then
		mkdir "${release_dir}"
		cp "${release_file}" "${release_dir}/"
		cd "${release_dir}/"

		sha256sum "GLogiK-${EXPECTED_VERSION}.tar.gz" > sha256sum
		sha512sum "GLogiK-${EXPECTED_VERSION}.tar.gz" > sha512sum
		whirlpool-hash "GLogiK-${EXPECTED_VERSION}.tar.gz" > whirlpool

		local GPGID="netbox253@gmail.com"

		gpg --detach-sign -o GLogiK-${EXPECTED_VERSION}.gpg -u ${GPGID} "${release_file}"
		for x in sha256sum sha512sum whirlpool; do
			gpg --detach-sign -o ${x}.gpg -u ${GPGID} ${x}
		done
	fi
}

make distcheck_it

if [ $? -eq 0 ] ; then
	do_release_dir
else
	printf "\n\nmake distcheck failed, do-release abort\n"
fi

