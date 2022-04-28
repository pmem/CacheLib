/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cachelib/allocator/CacheAllocatorConfig.h"
#include "cachelib/allocator/MemoryTierCacheConfig.h"
#include "cachelib/allocator/tests/TestBase.h"

namespace facebook {
namespace cachelib {

namespace tests {

using AllocatorT = LruAllocator;

class CacheAllocatorConfigTest : public testing::Test {};

TEST_F(CacheAllocatorConfigTest, MultipleTier0Config) {
  AllocatorT::Config config1, config2;
  // Throws if vector of tier configs is emptry
  EXPECT_THROW(
      config1.configureMemoryTiers(std::vector<MemoryTierCacheConfig>()),
      std::invalid_argument);
  EXPECT_THROW(config2.configureMemoryTiers(
                   123456, std::vector<MemoryTierCacheConfig>()),
               std::invalid_argument);
}

TEST_F(CacheAllocatorConfigTest, MultipleTier1Config) {
  AllocatorT::Config config1, config2;
  size_t cacheSize = 1234;
  // Accepts single-tier configuration
  config1.setCacheSize(cacheSize).configureMemoryTiers(
      {MemoryTierCacheConfig::fromShm().setSize(cacheSize)});
  config1.setCacheSize(0).configureMemoryTiers(
      {MemoryTierCacheConfig::fromShm().setSize(cacheSize)});
  config2.configureMemoryTiers(cacheSize,
                               {MemoryTierCacheConfig::fromShm().setRatio(1)});
}

TEST_F(CacheAllocatorConfigTest, MultipleTier2Config) {
  // TODO: Re-work this test when multiple-tier configuration
  // is enabled
  AllocatorT::Config config1, config2;
  // Throws if vector of tier configs > 1
  EXPECT_THROW(config1.configureMemoryTiers(
                   {MemoryTierCacheConfig::fromShm().setSize(1234),
                    MemoryTierCacheConfig::fromShm().setSize(1234)}),
               std::invalid_argument);
  EXPECT_THROW(config2.configureMemoryTiers(
                   123456,
                   {MemoryTierCacheConfig::fromShm().setRatio(1),
                    MemoryTierCacheConfig::fromShm().setRatio(1)}),
               std::invalid_argument);
}

} // namespace tests
} // namespace cachelib
} // namespace facebook
