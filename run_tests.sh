#!/bin/bash

function upload_codecov() {
	printf "\n$(tput setaf 1)$(tput setab 7)COVERAGE ${FUNCNAME[0]} START$(tput sgr 0)\n"
	gcovexe="gcov"

	# run gcov exe, using their bash (remove parsed coverage files, set flag and exit 1 if not successful)
	# we rely on parsed report on codecov.io; the output is quite long, hence it's disabled using -X flag
	/opt/scripts/codecov -c -F ${1} -Z -x "${gcovexe}" -X "gcovout"

	printf "check for any leftover gcov files\n"
	leftover_files=$(find . -name "*.gcov")
	if [[ -n "${leftover_files}" ]]; then
		# display found files and exit with error (they all should be parsed)
		echo "${leftover_files}"
		return 1
	fi

	printf "$(tput setaf 1)$(tput setab 7)COVERAGE ${FUNCNAME[0]} END$(tput sgr 0)\n\n"
}

# Newline separated list of tests to ignore
BLACKLIST="allocator-test-AllocationClassTest
allocator-test-NvmCacheTests
common-test-TimeTests
common-test-UtilTests
shm-test-test_page_size"

if [ "$1" == "long" ]; then
    find -type f -executable | grep -vF "$BLACKLIST" | xargs -n1 bash -c
    upload_codecov cachelib_long
else
    find -type f \( -not -name "*bench*" -and -not -name "navy*" \) -executable | grep -vF "$BLACKLIST" | xargs -n1 bash -c
    upload_codecov cachelib_regular
fi
