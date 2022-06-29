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



namespace facebook {
namespace cachelib {


template <typename CacheT>
BackgroundEvictor<CacheT>::BackgroundEvictor(Cache& cache,
                               std::shared_ptr<BackgroundEvictorStrategy> strategy,
                               unsigned int tid)
    : cache_(cache),
      strategy_(strategy),
      tid_(tid) {
}

template <typename CacheT>
BackgroundEvictor<CacheT>::~BackgroundEvictor() { stop(std::chrono::seconds(0)); }

template <typename CacheT>
void BackgroundEvictor<CacheT>::work() {
  try {
      for (const auto pid : cache_.getRegularPoolIds()) {
        checkAndRun(pid);
      }
  } catch (const std::exception& ex) {
    XLOGF(ERR, "BackgroundEvictor interrupted due to exception: {}", ex.what());
  }
}

// Look for classes that exceed the target memory capacity
// and return those for eviction
template <typename CacheT>
void BackgroundEvictor<CacheT>::checkAndRun(PoolId pid) {    
  const auto& mpStats = cache_.getPoolByTid(pid,tid_).getStats();
  unsigned int evictions = 0;
  unsigned int classes = 0;
  for (auto& cid : mpStats.classIds) {
      classes++;
      auto batch = strategy_->calculateBatchSize(cache_,tid_,pid,cid);
      if (!batch) {
        continue;
      }

      stats.evictionSize.add(batch * mpStats.acStats.at(cid).allocSize);
    
      //try evicting BATCH items from the class in order to reach free target
      auto evicted =
          BackgroundEvictorAPIWrapper<CacheT>::traverseAndEvictItems(cache_,
              tid_,pid,cid,batch);
      evictions += evicted;

      const size_t cid_id = (size_t)mpStats.acStats.at(cid).allocSize;
      auto it = evictions_per_class_.find(cid_id);
      if (it != evictions_per_class_.end()) {
          it->second += evicted;
      } else {
          evictions_per_class_[cid_id] = 0;
      }
  }
  stats.numTraversals.inc();
  stats.numEvictedItems.add(evictions);
  stats.totalClasses.add(classes);
}

template <typename CacheT>
BackgroundEvictionStats BackgroundEvictor<CacheT>::getStats() const noexcept {
  BackgroundEvictionStats evicStats;
  evicStats.numEvictedItems = stats.numEvictedItems.get();
  evicStats.runCount = stats.numTraversals.get();
  evicStats.evictionSize = stats.evictionSize.get();
  evicStats.totalClasses = stats.totalClasses.get();

  return evicStats;
}

template <typename CacheT>
std::map<uint32_t,uint64_t> BackgroundEvictor<CacheT>::getClassStats() const noexcept {
  return evictions_per_class_;
}

} // namespace cachelib
} // namespace facebook
