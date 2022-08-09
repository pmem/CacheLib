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

namespace facebook {
namespace cachelib {

// Base class for background eviction strategy.
class BackgroundEvictorStrategy {

public:
  virtual std::vector<size_t> calculateBatchSizes(const CacheBase& cache,
                                       std::vector<std::tuple<TierId, PoolId, ClassId>> acVec) = 0;
};

} // namespace cachelib
} // namespace facebook
