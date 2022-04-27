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

TEST_F(CacheAllocatorConfigTest, MultipleTierConfig) {
  // TODO: Re-work this test when multiple-tier confioguration
  // is enabled
  AllocatorT::Config config;
  std::vector<MemoryTierCacheConfig> tierConfig;
  EXPECT_THROW(config.configureMemoryTiers(tierConfig), std::invalid_argument);
}

} // namespace tests
} // namespace cachelib
} // namespace facebook
