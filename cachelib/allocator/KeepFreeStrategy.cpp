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

#include "cachelib/allocator/KeepFreeStrategy.h"

#include <folly/logging/xlog.h>

namespace facebook {
namespace cachelib {


KeepFreeStrategy::KeepFreeStrategy(unsigned int nKeepFree)
   : nKeepFree_(nKeepFree) {} 

bool KeepFreeStrategy::shouldEvict(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid ) {

    const auto& mpStats = cache.getPoolByTid(pid,tid).getStats();
    size_t allocSize = mpStats.acStats.at(cid).allocSize;
    size_t totalAllocs = mpStats.acStats.at(cid).getTotalMemory() / allocSize;
    size_t freeAllocs = mpStats.acStats.at(cid).getTotalFreeMemory() / allocSize;
    if (freeAllocs < nKeepFree_) {
        return true;
    }
    return false;
}

unsigned int KeepFreeStrategy::calculateBatchSize(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid ) {
  const auto& mpStats = cache.getPoolByTid(pid,tid).getStats();
  size_t allocSize = mpStats.acStats.at(cid).allocSize;
  size_t freeAllocs = mpStats.acStats.at(cid).getTotalFreeMemory() / allocSize;

  return (unsigned int)(nKeepFree_ - freeAllocs);
}


} // namespace cachelib
} // namespace facebook
