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
class DynamicFreeThresholdStrategy : public BackgroundEvictorStrategy {

public:
  DynamicFreeThresholdStrategy(double lowEvictionAcWatermark, double highEvictionAcWatermark,  double highEvictionAcWatermarkPreviousStart, double highEvictionAcWatermarkPreviousEnd, uint64_t evictionHotnessThreshold, double previousBenefitMig, double currentBenefitMig, double toFreeMemPercent);
  ~DynamicFreeThresholdStrategy() {}

  size_t calculateBatchSize(const CacheBase& cache,
                                       unsigned int tid,
                                       PoolId pid,
                                       ClassId cid,
                                       size_t allocSize,
                                       size_t acMemorySize);
private:
  double lowEvictionAcWatermark{2.0}; //this threshold is used outside this class and is not adjusted currently
  //double lowEvictionAcWatermarkPreviousStart{2.0};
  //double lowEvictionAcWatermarkPreviousEnd{2.0}; 
  double highEvictionAcWatermark{5.0}; //this threshold is adjusted internally within this class
  double highEvictionAcWatermarkPreviousStart{5.0};
  double highEvictionAcWatermarkPreviousEnd{5.0};
  uint64_t evictionHotnessThreshold{40};
  double previousBenefitMig{0.0};
  double currentBenefitMig{0.0};
  double toFreeMemPercent{0.0}; //Q: What happens to this value when the background thread is not activated in a certain period in Class x? Should we set it to 0?

  double calculateBenefitMig();
};

} // namespace cachelib
} // namespace facebook
