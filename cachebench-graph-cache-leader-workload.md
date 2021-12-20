# Cachebench Graph Cache Leader Workload

This document explains how to build cachebench and run the cache graph leader workloads

Configs have been added for benchmarking DRAM only, DRAM + NVM, and eventually DRAM + PMEM

We use this workload to simulate Tao graph cache leader traffic with different cache configurations

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

# Running Graph Cache Leader Workloads

json config files are located at: [cachelib/cachebench/test_configs/ssd_perf/graph_cache_leader/](cachelib/cachebench/ssd_perf/graph_cache_leader/)


```
cd cachelib/cachebench/test_configs/ssd_perf/graph_cache_leader

../../../../opt/cachelib/bin/cachebench --json_test_config ./config-4G-DRAM.json --report_api_latency -progress_stats_file <output_file> -progress 10
```

# Tunables

The graph cache leader workload can be tuned via the json config

```CacheSizeMB``` : specifices the size of the in memory cache 

```nvmCacheSizeMB``` : specifices the size of the NVM cache

```nvmCachePaths``` : specifices the device to use for NVM cache

```numThreads``` : specifies number of cachebench threads to run as part of the workload generator

```numOps``` : specifies the number of operations to execute. This can be used to change the runtime of the workload


