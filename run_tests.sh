BLACKLIST="allocator-test-NvmCacheTests
	common-test-TimeTests
	shm-test-test_page_size"

find -type f \( -not -name "*bench*" \) -executable | grep -vF "$BLACKLIST" | xargs -n1 bash -c
