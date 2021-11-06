/*
 * Copyright (c) Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

#include "cachelib/shm/ShmCommon.h"

namespace facebook {
namespace cachelib {
class MemoryTierCacheConfig {
public:
  // Creates instance of MemoryTierCacheConfig for file-backed memory.
  // @param path to file which CacheLib will use to map memory from.
  // TODO: add fromDirectory, fromAnonymousMemory
  static MemoryTierCacheConfig fromFile(const std::string& _file) {
    MemoryTierCacheConfig config;
    config.shmOpts = FileShmSegmentOpts(_file);
    return config;
  }

  // Creates instance of MemoryTierCacheConfig for Posix/SysV Shared memory.
  static MemoryTierCacheConfig fromShm() {
    MemoryTierCacheConfig config;
    config.shmOpts = PosixSysVSegmentOpts();
    return config;
  }

  // Specifies size of this memory tier. Sizes of tiers  must be specified by
  // either setting size explicitly or using ratio, mixing of the two is not supported.
  MemoryTierCacheConfig& setSize(size_t _size) {
    size = _size;
    return *this;
  }

  // Specifies ratio of this memory tier to other tiers. Absolute size
  // of each tier can be calculated as:
  // cacheSize * tierRatio / Sum of ratios for all tiers; the difference
  // between total cache size and sum of all tier sizes resulted from
  // round off error is accounted for when calculating the last tier's
  // size to make the totals equal.
  MemoryTierCacheConfig& setRatio(double _ratio) {
    ratio = _ratio;
    return *this;
  }

  size_t getRatio() const noexcept { return ratio; }

  size_t getSize() const noexcept { return size; }

  const ShmTypeOpts& getShmTypeOpts() const noexcept { return shmOpts; }

  // Size of this memory tiers
  size_t size{0};

  // Ratio is a number of parts of the total cache size to be allocated for this tier.
  // E.g. if X is a total cache size, Yi are ratios specified for memory tiers,
  // then size of the i-th tier Xi = (X / (Y1 + Y2)) * Yi and X = sum(Xi)
  size_t ratio{0};

  // Options specific to shm type
  ShmTypeOpts shmOpts;

private:
  MemoryTierCacheConfig() = default;
};
} // namespace cachelib
} // namespace facebook
