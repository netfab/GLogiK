#!/bin/bash

function run_jq()
{
	local ret=$(command -v 'jq')
	if [ -z "${ret}" ]; then
		printf "jq utility not found in PATH\n"
		exit 7
	fi

	local -r headers=(
		"callback"
		"GKDBusEvent"
		"SIGas2v"
		"SIGb2v"
		"SIGq2v"
		"SIGs2as"
		"SIGs2b"
		"SIGs2D"
#		"SIGs2s"
		"SIGs2v"
		"SIGsG2v"
		"SIGsGM2v"
		"SIGsm2v"
		"SIGss2aG"
		"SIGss2am"
		"SIGss2aP"
		"SIGss2b"
		"SIGss2s"
		"SIGss2v"
		"SIGssyt2b"
		"SIGssyyy2b"
		"SIGv2v"
	)

	jq -n --tab '
		[
			$ARGS.positional[] | {
				include: [
					("\"lib/dbus/events/" + . + ".hpp\""), "private",
					"\"lib/dbus/GKDBus.hpp\"", "public"
				]
			}
		]
	' --args "${headers[@]}" > GKDBus.imp

	ret=$?
	printf "jq return code: ${ret}\n"
	exit ${ret}
}

run_jq
