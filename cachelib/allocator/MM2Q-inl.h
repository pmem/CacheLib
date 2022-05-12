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

namespace facebook {
namespace cachelib {

///* Container Interface Implementation */
template <typename T, MM2Q::Hook<T> T::*HookPtr>
MM2Q::Container<T, HookPtr>::Container(const serialization::MM2QObject& object,
                                       PtrCompressor compressor)
          : compressor_(std::move(compressor)),
            lruHot_(compressor_),
            lruWarm_(compressor_),
            lruWarmTail_(compressor_),
            lruCold_(compressor_),
            lruColdTail_(compressor_),
            tailTrackingEnabled_(*object.tailTrackingEnabled_ref()),
            config_(*object.config_ref()) {
  
  lruRefreshTime_ = config_.lruRefreshTime;
  nextReconfigureTime_ = config_.mmReconfigureIntervalSecs.count() == 0
                             ? std::numeric_limits<Time>::max()
                             : static_cast<Time>(util::getCurrentTimeSec()) +
                                   config_.mmReconfigureIntervalSecs.count();
    //lruHot_(std::move(compressor));
    //lruWarm_(std::move(compressor));
    //lruWarmTail_(std::move(compressor));
    //lruCold_(std::move(compressor));
    //lruColdTail_(std::move(compressor));

}
template <typename T, MM2Q::Hook<T> T::*HookPtr>
MM2Q::Container<T, HookPtr>::Container(Config c, PtrCompressor compressor)

          : compressor_(std::move(compressor)),
            lruHot_(compressor_),
            lruWarm_(compressor_),
            lruWarmTail_(compressor_),
            lruCold_(compressor_),
            lruColdTail_(compressor_),
            tailTrackingEnabled_(c.tailSize > 0),
            config_(std::move(c)) {
     
      //lruHot_(std::move(compressor));
      //lruWarm_(std::move(compressor));
      //lruWarmTail_(std::move(compressor));
      //lruCold_(std::move(compressor));
      //lruColdTail_(std::move(compressor));

      lruRefreshTime_ = config_.lruRefreshTime;
      nextReconfigureTime_ =
          config_.mmReconfigureIntervalSecs.count() == 0
              ? std::numeric_limits<Time>::max()
              : static_cast<Time>(util::getCurrentTimeSec()) +
                    config_.mmReconfigureIntervalSecs.count();
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
bool MM2Q::Container<T, HookPtr>::recordAccess(T& node,
                                               AccessMode mode) noexcept {
  if ((mode == AccessMode::kWrite && !config_.updateOnWrite) ||
      (mode == AccessMode::kRead && !config_.updateOnRead)) {
    return false;
  }

  const auto curr = static_cast<Time>(util::getCurrentTimeSec());
  // check if the node is still being memory managed
  if (node.isInMMContainer() &&
      ((curr >= getUpdateTime(node) +
                    lruRefreshTime_.load(std::memory_order_relaxed)))) {
    
    //logic is as follows:
    // 1. if hot 
    //      a. move object to head
    // 2. if cold OR cold tail
    //      a. unmark from tail and/or cold
    //      b. remove and put in WARM
    //      c. call rebalance??
    // 3. if warm tail
    //      a. unmark from tail
    //      b. move to head of warm
    // 4. if warm
    //      a. move to head
    //
    // cases (1) & (4) require only 1 lock (curr LRU q)
    // case (3) requires warm and warm tail locks
    // case (2) requires cold/cold-tail and warm lock
    
    if (isHot(node)) {
        std::scoped_lock lock(mutexHot_);
        lruHot_.moveToHead(node);
        ++numHotAccesses_;
        setUpdateTime(node, curr);
    } else if (isCold(node) && inTail(node)) {
        std::scoped_lock lock(mutexColdTail_,mutexWarm_);
        unmarkTail(node);
        ++numColdTailAccesses_;
        lruColdTail_.remove(node);
        lruWarm_.linkAtHead(node);
        unmarkCold(node);
        ++numColdAccesses_;
        setUpdateTime(node, curr);
    } else if (isCold(node)) {
        std::scoped_lock lock(mutexCold_,mutexWarm_);
        lruCold_.remove(node);
        lruWarm_.linkAtHead(node);
        unmarkCold(node);
        ++numColdAccesses_;
        setUpdateTime(node, curr);
    } else if (isWarm(node) && inTail(node)) {
        std::scoped_lock lock(mutexWarmTail_,mutexWarm_);
        unmarkTail(node);
        ++numWarmTailAccesses_;
        lruWarmTail_.remove(node);
        lruWarm_.linkAtHead(node);
        ++numWarmAccesses_;
        setUpdateTime(node, curr);
    } else if (isWarm(node)) {
        std::scoped_lock lock(mutexWarm_);
        lruWarm_.moveToHead(node);
        ++numWarmAccesses_;
        setUpdateTime(node, curr);
    }
    return true;

  }
  return false;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat MM2Q::Container<T, HookPtr>::getEvictionAgeStat(
    uint64_t projectedLength) const noexcept {
    std::lock(mutexHot_,mutexWarm_,mutexWarmTail_,mutexCold_,mutexColdTail_);
    return getEvictionAgeStatLocked(projectedLength);
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat MM2Q::Container<T, HookPtr>::getEvictionAgeStatLocked(
    uint64_t projectedLength) const noexcept {
  EvictionAgeStat stat;
  const auto curr = static_cast<Time>(util::getCurrentTimeSec());

  stat.hotQueueStat.oldestElementAge = getOldestAgeLocked(LruType::Hot, curr);
  stat.hotQueueStat.size = lruHot_.size();

  // sum the tail and the main list for the ones that have tail.
  stat.warmQueueStat.oldestElementAge =
      getOldestAgeLocked(LruType::WarmTail, curr);
  stat.warmQueueStat.size = lruWarm_.size() +
                            lruWarmTail_.size();

  stat.coldQueueStat.oldestElementAge =
      getOldestAgeLocked(LruType::ColdTail, curr);
  stat.coldQueueStat.size = lruColdTail_.size() +
                            lruCold_.size();

  auto it = lruWarmTail_.rbegin();
  for (size_t numSeen = 0; numSeen < projectedLength && it != lruWarmTail_.rend();
       ++numSeen, ++it) {
  }
  stat.projectedAge = (it != lruWarmTail_.rend()) ? curr - getUpdateTime(*it)
                                          : stat.warmQueueStat.oldestElementAge;
  return stat;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
uint32_t MM2Q::Container<T, HookPtr>::getOldestAgeLocked(
    LruType lruType, Time currentTime) const noexcept {
    
    switch (lruType) {
    case LruType::Hot:
      return lruHot_.rbegin() != lruHot_.rend() ? 
          currentTime - getUpdateTime(*lruHot_.rbegin()) : 0;
    case LruType::ColdTail:
      return lruColdTail_.rbegin() != lruColdTail_.rend() ? 
          currentTime - getUpdateTime(*lruColdTail_.rbegin()) : 0;
    case LruType::Cold:
      return lruCold_.rbegin() != lruCold_.rend() ? 
          currentTime - getUpdateTime(*lruCold_.rbegin()) : 0;
    case LruType::WarmTail:
      return lruWarmTail_.rbegin() != lruWarmTail_.rend() ? 
          currentTime - getUpdateTime(*lruWarmTail_.rbegin()) : 0;
    case LruType::Warm:
      return lruWarm_.rbegin() != lruWarm_.rend() ? 
          currentTime - getUpdateTime(*lruWarm_.begin()) : 0;
    case LruType::NumTypes:
      XDCHECK(false);
    }
    return 0;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
typename MM2Q::LruType MM2Q::Container<T, HookPtr>::getLruType(
    const T& node) const noexcept {
  if (isHot(node)) {
    return LruType::Hot;
  }
  if (isCold(node)) {
    return inTail(node) ? LruType::ColdTail : LruType::Cold;
  }
  return inTail(node) ? LruType::WarmTail : LruType::Warm;
}


template <typename T, MM2Q::Hook<T> T::*HookPtr>
std::mutex* MM2Q::Container<T, HookPtr>::getMutex(
    const T& node) noexcept {
  if (isHot(node)) {
    return &mutexHot_;
  }
  if (isCold(node)) {
    return inTail(node) ? &mutexColdTail_ : &mutexCold_;
  }
  return inTail(node) ? &mutexWarmTail_ : &mutexWarm_;
}

//make probablistic and acquire locks here
//
template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::rebalance() noexcept {
    
  std::scoped_lock lock(mutexHot_,mutexWarm_,mutexWarmTail_,mutexCold_,mutexColdTail_);
  // shrink Warm (and WarmTail) if their total size is larger than expected
  size_t lru_size = lruHot_.size() +
                    lruWarm_.size() +
                    lruWarmTail_.size() +
                    lruCold_.size() +
                    lruColdTail_.size();

  size_t expectedSize = config_.getWarmSizePercent() * lru_size / 100;
  while (lruWarm_.size() +
             lruWarmTail_.size() >
         expectedSize) {
    //auto popFrom = [&](LruType lruType) -> T* {
    //  
    //  auto lru = *getLruFromType(lruType);
    //  T* node = lru.getTail();
    //  XDCHECK(node);
    //  lru.remove(*node);
    //  if (lruType == WarmTail || lruType == ColdTail) {
    //    unmarkTail(*node);
    //  }
    //  return node;
    //};
    auto popFromWarmTail = [&](LruType lruType) -> T* {
      
      T* node = lruWarmTail_.getTail();
      XDCHECK(node);
      lruWarmTail_.remove(*node);
      unmarkTail(*node);
      return node;
    };
    auto popFromWarm = [&](LruType lruType) -> T* {
      
      T* node = lruWarm_.getTail();
      XDCHECK(node);
      lruWarm_.remove(*node);
      return node;
    };
    // remove from warm tail if it is not empty. if empty, remove from warm.
    T* node = lruWarmTail_.size() > 0
                  ? popFromWarmTail(LruType::WarmTail)
                  : popFromWarm(LruType::Warm);
    XDCHECK(isWarm(*node));
    lruCold_.linkAtHead(*node);
    markCold(*node);
  }

  // shrink Hot if its size is larger than expected
  expectedSize = config_.hotSizePercent * lru_size / 100;
  while (lruHot_.size() > expectedSize) {
    auto node = lruHot_.getTail();
    XDCHECK(node);
    XDCHECK(isHot(*node));
    lruHot_.remove(*node);
    lruCold_.linkAtHead(*node);
    unmarkHot(*node);
    markCold(*node);
  }

  // adjust tail sizes for Cold and Warm
  adjustTail(LruType::Cold);
  adjustTail(LruType::Warm);
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
bool MM2Q::Container<T, HookPtr>::add(T& node) noexcept {
  const auto currTime = static_cast<Time>(util::getCurrentTimeSec());
  {
    std::scoped_lock lock(mutexHot_);
    if (node.isInMMContainer()) {
      return false;
    }

    markHot(node);
    unmarkCold(node);
    unmarkTail(node);
    lruHot_.linkAtHead(node);
    

    node.markInMMContainer();
    setUpdateTime(node, currTime);
  }
  int rebal = rand() % 100;
  if (rebal <= config_.rebalanceProb) {
    rebalance(); //TODO figure out when to call rebalance
  }
  return true;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
typename MM2Q::Container<T, HookPtr>::Iterator
MM2Q::Container<T, HookPtr>::getEvictionIterator() const noexcept {
  // we cannot use combined critical sections with folly::DistributedMutex here
  // because the lock is held for the lifetime of the eviction iterator.  In
  // other words, the abstraction of the iterator just does not lend itself well
  // to combinable critical sections as the user can hold the lock for an
  // arbitrary amount of time outside a lambda-friendly piece of code (eg. they
  // can return the iterator from functions, pass it to functions, etc)
  //
  // it would be theoretically possible to refactor this interface into
  // something like the following to allow combining
  //
  //    mm2q.withEvictionIterator([&](auto iterator) {
  //      // user code
  //    });
  //
  // at the time of writing it is unclear if the gains from combining are
  // reasonable justification for the codemod required to achieve combinability
  // as we don't expect this critical section to be the hotspot in user code.
  // This is however subject to change at some time in the future as and when
  // this assertion becomes false.

  if (config_.tailSize > 0) {
    std::unique_lock<Mutex> l(mutexColdTail_);
    return Iterator{std::move(l), lruColdTail_.rbegin()};
  } else {
    std::unique_lock<Mutex> l(mutexCold_);
    return Iterator{std::move(l), lruCold_.rbegin()};
  }
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::removeLocked(T& node) noexcept {
  LruType type = getLruType(node);
  switch (type) {
  case LruType::Hot:
    lruHot_.remove(node);
    break;
  case LruType::ColdTail:
    lruColdTail_.remove(node); 
    break;
  case LruType::Cold:
    lruCold_.remove(node); 
    break;
  case LruType::WarmTail:
    lruWarmTail_.remove(node);
    break; // warm is indicated by not marking hot or cold
  case LruType::Warm:
    lruWarm_.remove(node);
    break;
  case LruType::NumTypes:
    XDCHECK(false);
  }
  node.unmarkInMMContainer();
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::setConfig(const Config& newConfig) {
  if (!tailTrackingEnabled_ && newConfig.tailSize > 0) {
    throw std::invalid_argument(
        "Cannot turn on tailHitsTracking (cache drop needed)");
  }
  if (tailTrackingEnabled_ && !newConfig.tailSize) {
    throw std::invalid_argument(
        "Cannot turn off tailHitsTracking (cache drop needed)");
  }

    std::scoped_lock lock(mutexHot_,mutexWarm_,mutexWarmTail_,mutexCold_,mutexColdTail_);
    config_ = newConfig;
    lruRefreshTime_.store(config_.lruRefreshTime, std::memory_order_relaxed);
    nextReconfigureTime_ = config_.mmReconfigureIntervalSecs.count() == 0
                               ? std::numeric_limits<Time>::max()
                               : static_cast<Time>(util::getCurrentTimeSec()) +
                                     config_.mmReconfigureIntervalSecs.count();
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
typename MM2Q::Config MM2Q::Container<T, HookPtr>::getConfig() const {
  //return lruMutex_->lock_combine([this]() { return config_; });

    std::scoped_lock lock(mutexHot_,mutexWarm_,mutexWarmTail_,mutexCold_,mutexColdTail_);
    return config_;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
bool MM2Q::Container<T, HookPtr>::remove(T& node) noexcept {
  {
    std::scoped_lock lock(*getMutex(node)); 
    if (!node.isInMMContainer()) {
      return false;
    }
    removeLocked(node);

  }
  int rebal = rand() % 100;
  if (rebal <= config_.rebalanceProb) {
    rebalance(); //TODO figure out when to call rebalance
  }
  //if (doRebalance) {
  //  rebalance();
  //}
  return true;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::remove(Iterator& it) noexcept {
  T& node = *it;
  XDCHECK(node.isInMMContainer());
  ++it;
  // rebalance should not be triggered inside this remove, because changing the
  // queues while having the EvictionIterator can cause inconsistency problem
  // for the iterator. Also, this remove is followed by an insertion into the
  // same container, which will trigger rebalance.
  removeLocked(node); //, /* doRebalance = */ false);
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
bool MM2Q::Container<T, HookPtr>::replace(T& oldNode, T& newNode) noexcept {

    
  {
    std::scoped_lock lock(*getMutex(oldNode));

    if (!oldNode.isInMMContainer() || newNode.isInMMContainer()) {
      return false;
    }
    const auto updateTime = getUpdateTime(oldNode);

    LruType type = getLruType(oldNode);
    switch (type) {
    case LruType::Hot:
      lruHot_.replace(oldNode,newNode);
      markHot(newNode);
      break;
    case LruType::ColdTail:
      lruColdTail_.replace(oldNode,newNode);
      markTail(newNode); 
      markCold(newNode);
      break;
    case LruType::Cold:
      lruCold_.replace(oldNode,newNode);
      markCold(newNode);
      break;
    case LruType::WarmTail:
      lruWarmTail_.replace(oldNode,newNode);
      markTail(newNode);
      break; // warm is indicated by not marking hot or cold
    case LruType::Warm:
      lruWarm_.replace(oldNode,newNode);
      break;
    case LruType::NumTypes:
      XDCHECK(false);
    }

    oldNode.unmarkInMMContainer();
    newNode.markInMMContainer();
    setUpdateTime(newNode, updateTime);
    return true;
  }
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
MM2Q::LruType MM2Q::Container<T, HookPtr>::getTailLru(LruType list) const {
  switch (list) {
  case LruType::Warm:
    return LruType::WarmTail;
  case LruType::Cold:
    return LruType::ColdTail;
  default:
    XDCHECK(false);
    throw std::invalid_argument("The LRU list does not have a tail list");
  }
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::adjustTail(LruType type) {
  //auto adjustFrom = [&](LruList tail, LruList list) -> T* {
  //  auto ptr = list.getTail();
  //  while (ptr && tail.size() + 1 <= config_.tailSize) {
  //    markTail(*ptr);
  //    list.remove(*ptr);
  //    tail.linkAtHead(*ptr);
  //    ptr = list.getTail();
  //  }
  //};
  if (type == LruType::Warm) {
    auto ptr = lruWarm_.getTail();
    while (ptr && lruWarmTail_.size() + 1 <= config_.tailSize) {
      markTail(*ptr);
      lruWarm_.remove(*ptr);
      lruWarmTail_.linkAtHead(*ptr);
      ptr = lruWarm_.getTail();
    }
  } else if (type == LruType::Cold) {
    auto ptr = lruCold_.getTail();
    while (ptr && lruColdTail_.size() + 1 <= config_.tailSize) {
      markTail(*ptr);
      lruCold_.remove(*ptr);
      lruColdTail_.linkAtHead(*ptr);
      ptr = lruCold_.getTail();
    }
  }
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
serialization::MM2QObject MM2Q::Container<T, HookPtr>::saveState()
    const noexcept {
  serialization::MM2QConfig configObject;
  *configObject.lruRefreshTime_ref() = lruRefreshTime_;
  *configObject.lruRefreshRatio_ref() = config_.lruRefreshRatio;
  *configObject.updateOnWrite_ref() = config_.updateOnWrite;
  *configObject.updateOnRead_ref() = config_.updateOnRead;
  *configObject.hotSizePercent_ref() = config_.hotSizePercent;
  *configObject.coldSizePercent_ref() = config_.coldSizePercent;
  *configObject.rebalanceOnRecordAccess_ref() = config_.rebalanceOnRecordAccess;

  serialization::MM2QObject object;
  *object.config_ref() = configObject;
  *object.tailTrackingEnabled_ref() = tailTrackingEnabled_;
  //*object.lrus_ref() = lru_.saveState();
  return object;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
MMContainerStat MM2Q::Container<T, HookPtr>::getStats() const noexcept {
  //return lruMutex_->lock_combine([this]() {
  //  auto* tail = lru_.size() == 0 ? nullptr : lru_.rbegin().get();
  //  auto computeWeightedAccesses = [&](size_t warm, size_t cold) {
  //    return (warm * config_.getWarmSizePercent() +
  //            cold * config_.coldSizePercent) /
  //           100;
  //  };

    // note that in the analagous code in MMLru, we return an instance of
    // std::array<std::uint64_t, 6> and construct the MMContainerStat instance
    // outside the lock with some other data that is not required to be read
    // under a lock.  This is done there to take advantage of
    // folly::DistributedMutex's return-value inlining feature which coalesces
    // the write from the critical section with the atomic operations required
    // as part of the internal synchronization mechanism.  That then transfers
    // the synchronization signal as well as the return value in one single
    // cacheline invalidation message.  At the time of writing this decision was
    // based on an implementation-detail aware to the author - 48 bytes can be
    // coalesced with the synchronization signal in folly::DistributedMutex.
    //
    // we cannot do that here because this critical section returns more data
    // than can be coalesced internally by folly::DistributedMutex (> 48 bytes).
    // So we construct and return the entire object under the lock.
    return 
        MMContainerStat{
        0, //lru_.size(),
        false, //nullptr ? 0 : getUpdateTime(*tail),
        lruRefreshTime_.load(std::memory_order_relaxed),
        numHotAccesses_,
        numColdAccesses_,
        numWarmAccesses_,
        0 };
        //computeWeightedAccesses(numWarmTailAccesses_, numColdTailAccesses_)};
  
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::reconfigureLocked(const Time& currTime) {
  //if (currTime < nextReconfigureTime_) {
  //  return;
  //}
  //nextReconfigureTime_ = currTime + config_.mmReconfigureIntervalSecs.count();

  //// update LRU refresh time
  //auto stat = getEvictionAgeStatLocked(0);
  //auto lruRefreshTime = std::min(
  //    std::max(config_.defaultLruRefreshTime,
  //             static_cast<uint32_t>(stat.warmQueueStat.oldestElementAge *
  //                                   config_.lruRefreshRatio)),
  //    kLruRefreshTimeCap);
  //lruRefreshTime_.store(lruRefreshTime, std::memory_order_relaxed);
}

// Iterator Context Implementation
template <typename T, MM2Q::Hook<T> T::*HookPtr>
MM2Q::Container<T, HookPtr>::Iterator::Iterator(
    LockHolder l, const typename LruList::Iterator& iter) noexcept
    : LruList::Iterator(iter), l_(std::move(l)) {}
} // namespace cachelib
} // namespace facebook
