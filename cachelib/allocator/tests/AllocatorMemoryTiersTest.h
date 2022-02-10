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

#pragma once

#include "cachelib/allocator/CacheAllocatorConfig.h"
#include "cachelib/allocator/MemoryTierCacheConfig.h"
#include "cachelib/allocator/tests/TestBase.h"

namespace facebook {
namespace cachelib {
namespace tests {

template <typename AllocatorT>
class AllocatorMemoryTiersTest : public AllocatorTest<AllocatorT> {
 public:
  void configCache() {
    typename AllocatorT::Config config;
    config
        .setCacheSize(12 * Slab::kSize) // 12 * 4MB
        .setCacheName("MultiTier Cache")
        .enableCachePersistence("/tmp")
        .setAccessConfig({
          25 /* bucket power */,
          10 /* lock power */}) // assuming caching 20 million items
        .configureMemoryTiers({
            MemoryTierCacheConfig::fromShm().setRatio(1),
            MemoryTierCacheConfig::fromFile("/tmp/a" + std::to_string(::getpid())).setRatio(2)})
        .validate(); // throw if config is incorrect

    cache_ = std::make_unique<AllocatorT>(AllocatorT::SharedMemNew, config);
    pool_ = cache_->addPool("default", cache_->getCacheMemoryStats().cacheSize);
  }

  void destroyCache() { cache_.reset(); }

  typename AllocatorT::ItemHandle get(typename AllocatorT::Key key) { return cache_->find(key); }

  bool put(typename AllocatorT::Key key, const std::string& value) {
    auto handle = cache_->allocate(pool_, key, value.size());
    if (!handle) {
      return false; // cache may fail to evict due to too many pending writes
    }
    std::memcpy(handle->getWritableMemory(), value.data(), value.size());
    cache_->insertOrReplace(handle);
    return true;
  }

  void populateCache() {
    for(size_t i = 0; i < numEntries_; i++) {
      auto res = put(getKey(i), value_);
      ASSERT_TRUE(res);
    }
  }

  bool verifyCache() {
    for(size_t i = 0; i < numEntries_; ++i) {
      auto item = get(getKey(i));
      if (item) {
        folly::StringPiece sp{reinterpret_cast<const char*>(item->getMemory()),
                              item->getSize()};
        ASSERT_EQ(sp, value_);
      } else {
        return false;
      }
    }
    return true;
  }

 private:
  PoolId pool_;
  std::unique_ptr<AllocatorT> cache_;
  size_t numEntries_ = 13000;
  std::string value_ = std::string(4*1024, 'X'); // 4 KB value
  std::string getKey(size_t i) { return "key" + std::to_string(i); }
};
} // namespace tests
} // namespace cachelib
} // namespace facebook
