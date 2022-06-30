/*
 * Copyright (c) Intel and its affiliates.
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

#include "cachelib/allocator/FreeThresholdStrategy.h"

#include <folly/logging/xlog.h>

namespace facebook {
namespace cachelib {



FreeThresholdStrategy::FreeThresholdStrategy(double evictionSlabWatermark, double lowEvictionAcWatermark, double highEvictionAcWatermark, uint64_t evictionHotnessThreshold)
    : evictionSlabWatermark(evictionSlabWatermark), lowEvictionAcWatermark(lowEvictionAcWatermark), highEvictionAcWatermark(highEvictionAcWatermark), evictionHotnessThreshold(evictionHotnessThreshold) {}

size_t FreeThresholdStrategy::calculateBatchSize(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid,
                                       size_t numItems,
                                       size_t acMemorySize) {
  if (cache.slabsAllocatedPercentage(tid) < evictionSlabWatermark)
    return 0;

  auto acOccupied = cache.acAllocatedPercentage(tid, pid, cid);
  if (acOccupied <= highEvictionAcWatermark)
    return 0;

  auto toFreeMemPercent = acOccupied - highEvictionAcWatermark;
  auto toFreeItems = static_cast<size_t>(toFreeMemPercent * acMemorySize / allocSize);

  return toFreeItems;
}

} // namespace cachelib
} // namespace facebook
