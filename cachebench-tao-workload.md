# Cachebench Tao Workload

This document explains how to build cachebench and run the 'tao' in memory workload.

The workload configs are based on older versions of ssd-perf/graph-leader

We use this workload to simulate Tao graph cache leader traffic but with NVM cache disabled, so we only benchmark the in-memory cache performance.

# Building cachebench 

CacheLib provides a build script which prepares and installs all
dependencies and prerequisites, then builds CacheLib.
The build script has been tested to work on CentOS 8,
Ubuntu 18.04, and Debian 10.

```sh
git clone https://github.com/facebook/CacheLib
cd CacheLib
./contrib/build.sh -d -j -v

# The resulting library and executables:
./opt/cachelib/bin/cachebench --help
```

# Running Tao Workload

json config files are located at: [cachelib/cachebench/tesjconfigs/tao_inmemory/](cachelib/cachebench/test_configs/tao_inmemory/)


```
cd cachelib/cachebench/test_configs/tao_inmemory

../../../opt/cachelib/bin/cachebench --json_test_config --report_api_latency -progress_stats_file <file> -progress 10
```

# Tunables

The tao workload can be tuned via the json config

```CacheSizeMB``` : specifices the size of the in memory cache 

```numThreads``` : specifies number of cachebench threads to run as part of the workload generator

```numOps``` : specifies the number of operations to execute. This can be used to change the runtime of the workload



