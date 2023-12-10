#!/bin/bash

function check_binary() {
	if ! command -v "${1}" &> /dev/null
	then
		printf "${1} could not be found\n" 1>&2
		exit 1
	fi
}

check_binary cat
check_binary sed

cat VERSION | sed 's/GLogiK version //'
