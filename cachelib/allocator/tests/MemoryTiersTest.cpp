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

#include <numeric>

#include "cachelib/allocator/CacheAllocator.h"
#include "cachelib/allocator/tests/TestBase.h"

namespace facebook {
namespace cachelib {
namespace tests {

using LruAllocatorConfig = CacheAllocatorConfig<LruAllocator>;
using LruMemoryTierConfigs = LruAllocatorConfig::MemoryTierConfigs;
using Strings = std::vector<std::string>;

const size_t metaDataSize = 4194304;
constexpr size_t MB = 1024ULL * 1024ULL;
constexpr size_t GB = MB * 1024ULL;

using SizePair = std::tuple<size_t, size_t>;
using SizePairs = std::vector<SizePair>;

const size_t defaultTotalCacheSize{1 * GB};
const std::string defaultCacheDir{"/var/metadataDir"};
const std::string defaultPmemPath{"/dev/shm/p1"};
const std::string defaultDaxPath{"/dev/dax0.0"};

template <typename Allocator>
class MemoryTiersTest : public AllocatorTest<Allocator> {
 public:
  void basicCheck(LruAllocatorConfig& actualConfig,
                  const Strings& expectedPaths = {defaultPmemPath},
                  size_t expectedTotalCacheSize = defaultTotalCacheSize,
                  const std::string& expectedCacheDir = defaultCacheDir) {
    EXPECT_EQ(actualConfig.getCacheSize(), expectedTotalCacheSize);
    EXPECT_EQ(actualConfig.getMemoryTierConfigs().size(), expectedPaths.size());
    EXPECT_EQ(actualConfig.getCacheDir(), expectedCacheDir);
    auto configs = actualConfig.getMemoryTierConfigs();

    size_t sum_sizes = std::accumulate(
        configs.begin(), configs.end(), 0,
        [](const size_t i, const MemoryTierCacheConfig& config) {
          return i + config.getSize();
        });
    size_t sum_ratios = std::accumulate(
        configs.begin(), configs.end(), 0,
        [](const size_t i, const MemoryTierCacheConfig& config) {
          return i + config.getRatio();
        });

    size_t partition_size = 0;
    if (sum_ratios) {
      partition_size = actualConfig.getCacheSize() / sum_ratios;
      /* Sum of sizes can be lower due to rounding down to partition_size. */
      EXPECT_GE(sum_sizes, expectedTotalCacheSize - partition_size);
    }

    for (auto i = 0; i < configs.size(); ++i) {
      auto& opt = std::get<FileShmSegmentOpts>(configs[i].getShmTypeOpts());
      EXPECT_EQ(opt.path, expectedPaths[i]);
      EXPECT_GT(configs[i].getSize(), 0);
      if (configs[i].getRatio() && (i < configs.size() - 1)) {
        EXPECT_EQ(configs[i].getSize(), partition_size * configs[i].getRatio());
      }
    }
  }

  LruAllocatorConfig createTestCacheConfig(
      const Strings& tierPaths = {defaultPmemPath},
      const SizePairs& sizePairs = {std::make_tuple(1 /* ratio */,
                                                    0 /* size */)},
      bool setPosixForShm = true,
      size_t cacheSize = defaultTotalCacheSize,
      const std::string& cacheDir = defaultCacheDir) {
    LruAllocatorConfig cfg;
    cfg.setCacheSize(cacheSize).enableCachePersistence(cacheDir);

    if (setPosixForShm)
      cfg.usePosixForShm();

    LruMemoryTierConfigs tierConfigs;
    tierConfigs.reserve(tierPaths.size());
    for (auto i = 0; i < tierPaths.size(); ++i) {
      tierConfigs.push_back(MemoryTierCacheConfig::fromFile(tierPaths[i])
                                .setRatio(std::get<0>(sizePairs[i]))
                                .setSize(std::get<1>(sizePairs[i])));
    }
    cfg.configureMemoryTiers(tierConfigs);
    return cfg;
  }

  LruAllocatorConfig createTieredCacheConfig(size_t totalCacheSize,
                                             size_t numTiers = 2) {
    LruAllocatorConfig tieredCacheConfig{};
    std::vector<MemoryTierCacheConfig> configs;
    for (auto i = 1; i <= numTiers; ++i) {
      configs.push_back(MemoryTierCacheConfig::fromFile(
                            folly::sformat("/tmp/tier{}-{}", i, ::getpid()))
                            .setRatio(1));
    }
    tieredCacheConfig.setCacheSize(totalCacheSize)
        .enableCachePersistence(
            folly::sformat("/tmp/multi-tier-test/{}", ::getpid()))
        .usePosixForShm()
        .configureMemoryTiers(configs);
    return tieredCacheConfig;
  }

  LruAllocatorConfig createDramCacheConfig(size_t totalCacheSize) {
    LruAllocatorConfig dramConfig{};
    dramConfig.setCacheSize(totalCacheSize);
    return dramConfig;
  }

  void validatePoolSize(PoolId poolId,
                        std::unique_ptr<LruAllocator>& allocator,
                        size_t expectedSize) {
    size_t actualSize = allocator->getPoolSize(poolId);
    EXPECT_EQ(actualSize, expectedSize);
  }
};

using LruMemoryTiersTest = MemoryTiersTest<LruAllocator>;

TEST_F(LruMemoryTiersTest, TestValid1TierPmemRatioConfig) {
  LruAllocatorConfig cfg = createTestCacheConfig({defaultPmemPath});
  basicCheck(cfg);
}

TEST_F(LruMemoryTiersTest, TestValid1TierDaxRatioConfig) {
  LruAllocatorConfig cfg = createTestCacheConfig({defaultDaxPath});
  basicCheck(cfg, {defaultDaxPath});
}

TEST_F(LruMemoryTiersTest, TestValid1TierDaxSizeConfig) {
  LruAllocatorConfig cfg =
      createTestCacheConfig({defaultDaxPath},
                            {std::make_tuple(0, defaultTotalCacheSize)},
                            /* setPosixShm */ true,
                            /* cacheSize */ 0);
  basicCheck(cfg, {defaultDaxPath});

  // Setting size after conifguringMemoryTiers with sizes is not allowed.
  EXPECT_THROW(cfg.setCacheSize(defaultTotalCacheSize + 1),
               std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestValid2TierDaxPmemConfig) {
  LruAllocatorConfig cfg =
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 0), std::make_tuple(1, 0)});
  basicCheck(cfg, {defaultDaxPath, defaultPmemPath});
}

TEST_F(LruMemoryTiersTest, TestValid2TierDaxPmemRatioConfig) {
  LruAllocatorConfig cfg =
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(5, 0), std::make_tuple(2, 0)});
  basicCheck(cfg, {defaultDaxPath, defaultPmemPath});
}

TEST_F(LruMemoryTiersTest, TestValid2TierDaxPmemSizeConfig) {
  size_t size_1 = 4321, size_2 = 1234;
  LruAllocatorConfig cfg = createTestCacheConfig(
      {defaultDaxPath, defaultPmemPath},
      {std::make_tuple(0, size_1), std::make_tuple(0, size_2)}, true, 0);
  basicCheck(cfg, {defaultDaxPath, defaultPmemPath}, size_1 + size_2);

  // Setting size after conifguringMemoryTiers with sizes is not allowed.
  EXPECT_THROW(cfg.setCacheSize(size_1 + size_2 + 1), std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigPosixShmNotSet) {
  LruAllocatorConfig cfg =
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 0), std::make_tuple(1, 0)},
                            /* setPosixShm */ false);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigNumberOfPartitionsTooLarge) {
  EXPECT_THROW(createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                                     {std::make_tuple(defaultTotalCacheSize, 0),
                                      std::make_tuple(1, 0)})
                   .validate(),
               std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigSizesAndRatiosMixed) {
  EXPECT_THROW(
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 0), std::make_tuple(1, 1)}),
      std::invalid_argument);
  EXPECT_THROW(
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 1), std::make_tuple(0, 1)}),
      std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigSizesAndRatioNotSet) {
  EXPECT_THROW(
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 0), std::make_tuple(0, 0)}),
      std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigRatiosCacheSizeNotSet) {
  EXPECT_THROW(
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(1, 0), std::make_tuple(1, 0)},
                            /* setPosixShm */ true, /* cacheSize */ 0)
          .validate(),
      std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestInvalid2TierConfigSizesNeCacheSize) {
  EXPECT_THROW(
      createTestCacheConfig({defaultDaxPath, defaultPmemPath},
                            {std::make_tuple(0, 1), std::make_tuple(0, 1)}),
      std::invalid_argument);
}

TEST_F(LruMemoryTiersTest, TestPoolAllocations) {
  std::vector<size_t> totalCacheSizes = {48 * MB, 51 * MB, 256 * MB,
                                         1 * GB,  5 * GB,  8 * GB};
  std::vector<size_t> numTiers = {2, 3, 4};
  std::vector<SizePairs> sizePairs = {
      {},
      {std::make_tuple(1, 0)},
      {std::make_tuple(3, 0), std::make_tuple(2, 0)},
      {std::make_tuple(2, 0), std::make_tuple(2, 0), std::make_tuple(1, 0)},
      {std::make_tuple(1, 0), std::make_tuple(5, 0), std::make_tuple(1, 0),
       std::make_tuple(2, 0)}};
  const std::string path = "/tmp/tier";
  std::vector<Strings> paths = {
      {},
      {path + "0"},
      {path + "0", path + "1"},
      {path + "0", path + "1", path + "2"},
      {path + "0", path + "1", path + "2", path + "3"}};

  auto testAddPool = [&](LruAllocatorConfig& config, size_t poolSize,
                         bool isSizeValid = true, bool isTestMaxSize = false) {
    if (isTestMaxSize) {
      std::unique_ptr<LruAllocator> alloc = std::unique_ptr<LruAllocator>(
          new LruAllocator(LruAllocator::SharedMemNew, config));
      auto pool =
          alloc->addPool("maxPoolSize", alloc->getCacheMemoryStats().cacheSize);
      validatePoolSize(pool, alloc, alloc->getCacheMemoryStats().cacheSize);
    }
    std::unique_ptr<LruAllocator> alloc = std::unique_ptr<LruAllocator>(
        new LruAllocator(LruAllocator::SharedMemNew, config));
    if (isSizeValid) {
      auto pool = alloc->addPool("validPoolSize", poolSize);
      validatePoolSize(pool, alloc, poolSize);
    } else {
      EXPECT_THROW(alloc->addPool("invalidPoolSize", poolSize),
                   std::invalid_argument);
    }
  };

  for (auto nTiers : numTiers) {
    for (auto totalCacheSize : totalCacheSizes) {
      if (totalCacheSize <= nTiers * Slab::kSize * 4)
        continue;
      LruAllocatorConfig cfg =
          createTestCacheConfig(paths[nTiers], sizePairs[nTiers],
                                /* usePoisx */ true, totalCacheSize);
      basicCheck(cfg, paths[nTiers], totalCacheSize);
      testAddPool(cfg, 0, /* isSizeValid */ true, /* isTestMaxSize */ true);
      testAddPool(cfg, 1);
      testAddPool(cfg, totalCacheSize, /* isSizeValid */ false);
      testAddPool(cfg, totalCacheSize / nTiers);
    }
  }
}

} // namespace tests
} // namespace cachelib
} // namespace facebook
