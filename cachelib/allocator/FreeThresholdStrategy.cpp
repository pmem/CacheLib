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



FreeThresholdStrategy::FreeThresholdStrategy(double freeThreshold) 
    : freeThreshold_(freeThreshold) {}

bool FreeThresholdStrategy::shouldEvict(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid ) {

  const auto& mpStats = cache.getPoolByTid(pid,tid).getStats();
  size_t totalMem = mpStats.acStats.at(cid).getTotalMemory(); 
  size_t freeMem = mpStats.acStats.at(cid).getTotalFreeMemory();

  double currFree = (double)freeMem/(double)totalMem;
  if (currFree < freeThreshold_) {
      return true;
  } else {
      return false;
  }

}

unsigned int FreeThresholdStrategy::calculateBatchSize(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid ) {
  const auto& mpStats = cache.getPoolByTid(pid,tid).getStats();
  size_t totalMem = mpStats.acStats.at(cid).getTotalMemory(); 
  size_t freeMem = mpStats.acStats.at(cid).getTotalFreeMemory();

  size_t targetMem = (freeThreshold_ * totalMem) - freeMem;
  unsigned int batch = (targetMem / mpStats.acStats.at(cid).allocSize);

  return batch;
}


} // namespace cachelib
} // namespace facebook
