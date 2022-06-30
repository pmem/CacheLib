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

#include "cachelib/allocator/Cache.h"
#include "cachelib/allocator/BackgroundEvictorStrategy.h"

namespace facebook {
namespace cachelib {


// Base class for background eviction strategy.
class PromotionStrategy : public BackgroundEvictorStrategy {

public:
  PromotionStrategy(double promotionAcWatermark, uint64_t evictionHotnessThreshold):
  promotionAcWatermark(promotionAcWatermark), evictionHotnessThreshold(evictionHotnessThreshold)
  {

  }
  ~PromotionStrategy() {}

  size_t calculateBatchSize(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid,
                                       size_t allocSize,
                                       size_t memorySize) {
    XDCHECK(tid > 0);
    if (cache.slabsAllocatedPercentage(tid - 1) > promotionAcWatermark)
      return 0;

    // TODO: we can try to adjust it to not evict to many elements
    // but is it worth it...?
    return evictionHotnessThreshold;
  }
private:
  double promotionAcWatermark{97.0};
  uint64_t evictionHotnessThreshold;
};

} // namespace cachelib
} // namespace facebook
