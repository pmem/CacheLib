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
     : tailTrackingEnabled_(*object.tailTrackingEnabled_ref()),
      config_(*object.config_ref()) {
  
  //lru_ = std::vector<LruList>();
  lruRefreshTime_ = config_.lruRefreshTime;
  nextReconfigureTime_ = config_.mmReconfigureIntervalSecs.count() == 0
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
    
    //auto func = [&]() {
    //  reconfigureLocked(curr);
    //  if (!node.isInMMContainer()) {
    //    return false;
    //  }
    //  if (isHot(node)) {
    //    lru_[LruType::Hot).moveToHead(node);
    //    ++numHotAccesses_;
    //  } else if (isCold(node)) {
    //    if (inTail(node)) {
    //      unmarkTail(node);
    //      lru_[LruType::ColdTail).remove(node);
    //      ++numColdTailAccesses_;
    //    } else {
    //      lru_[LruType::Cold).remove(node);
    //    }
    //    lru_[LruType::Warm).linkAtHead(node);
    //    unmarkCold(node);
    //    ++numColdAccesses_;
    //    // only rebalance if config says so. recordAccess is called mostly on
    //    // latency sensitive cache get operations.
    //    if (config_.rebalanceOnRecordAccess) {
    //      rebalance();
    //    }
    //  } else {
    //    if (inTail(node)) {
    //      unmarkTail(node);
    //      lru_[LruType::WarmTail).remove(node);
    //      lru_[LruType::Warm).linkAtHead(node);
    //      ++numWarmTailAccesses_;
    //    } else {
    //      lru_[LruType::Warm).moveToHead(node);
    //    }
    //    ++numWarmAccesses_;
    //  }
    //  setUpdateTime(node, curr);
    //  return true;
    //};

    //auto hotFunc = [&]() {
    //    lru_[LruType::Hot].moveToHead(node);
    //    ++numHotAccesses_;
    //    setUpdateTime(node, curr);
    //}
    //auto coldFunc = [&]() {
    //    lru_[LruType::Cold].remove(node);
    //    lru_[LruType::Warm].linkAtHead(node);
    //    
    //    unmarkCold(node);
    //    ++numColdAccesses_;
    //    setUpdateTime(node, curr);
    //}
    
    //auto coldTailFunc = [&]() {
    //    
    //    unmarkTail(node);
    //    ++numColdTailAccesses_;
    //    lru_[LruType::ColdTail].remove(node);
    //    lru_[LruType::Warm].linkAtHead(node);
    //    
    //    unmarkCold(node);
    //    ++numColdAccesses_;
    //    setUpdateTime(node, curr);
    //}
    
    //auto warmTailFunc = [&]() {
    //    
    //    unmarkTail(node);
    //    ++numWarmTailAccesses_;
    //    lru_[LruType::WarmTail].remove(node);
    //    lru_[LruType::Warm].linkAtHead(node);
    //    
    //    ++numWarmAccesses_;
    //    setUpdateTime(node, curr);
    //}
    //
    //auto warmFunc = [&]() {
    //    lru_[LruType::Warm].moveToHead(node);
    //    ++numWarmAccesses_;
    //    setUpdateTime(node, curr);
    //}

    // if the tryLockUpdate optimization is on, and we were able to grab the
    // lock, execute the critical section and return true, else return false
    //
    // if the tryLockUpdate optimization is off, we always execute the critical
    // section and return true
    //if (config_.tryLockUpdate) {
    //  if (auto lck = LockHolder{*lruMutex_, std::try_to_lock}) {
    //    return func();
    //  }
    //  return false;
    //}

    //auto hotlck = std::unique_lock<Mutex>(lruMutex_[LruType::Hot], std::defer_lock);
    //auto warmlck = LockHolder{*lruMutex_[LruType::Warm], std::defer_lock};
    //auto warmtaillck = LockHolder{*lruMutex_[LruType::WarmTail], std::defer_lock};
    //auto coldlck = LockHolder{*lruMutex_[LruType::Cold], std::defer_lock};
    //auto coldtaillck = LockHolder{*lruMutex_[LruType::ColdTail], std::defer_lock};
    if (isHot(node)) {
        //std::lock(lruMutex_[LruType::Hot]);
        //std::scoped_lock lock(hotlck);
        std::scoped_lock lock(lruMutex_[LruType::Hot]);
        lru_[LruType::Hot].moveToHead(node);
        ++numHotAccesses_;
        setUpdateTime(node, curr);
    } else if (isCold(node) && inTail(node)) {
        std::scoped_lock lock(lruMutex_[LruType::ColdTail],lruMutex_[LruType::Warm]);
        unmarkTail(node);
        ++numColdTailAccesses_;
        lru_[LruType::ColdTail].remove(node);
        lru_[LruType::Warm].linkAtHead(node);
        unmarkCold(node);
        ++numColdAccesses_;
        setUpdateTime(node, curr);
    } else if (isCold(node)) {
        std::scoped_lock lock(lruMutex_[LruType::Cold],lruMutex_[LruType::Warm]);
        lru_[LruType::Cold].remove(node);
        lru_[LruType::Warm].linkAtHead(node);
        unmarkCold(node);
        ++numColdAccesses_;
        setUpdateTime(node, curr);
    } else if (isWarm(node) && inTail(node)) {
        std::scoped_lock lock(lruMutex_[LruType::WarmTail],lruMutex_[LruType::Warm]);
        unmarkTail(node);
        ++numWarmTailAccesses_;
        lru_[LruType::WarmTail].remove(node);
        lru_[LruType::Warm].linkAtHead(node);
        
        ++numWarmAccesses_;
        setUpdateTime(node, curr);
    } else if (isWarm(node)) {
        std::scoped_lock lock(lruMutex_[LruType::Warm]);
        lru_[LruType::Warm].moveToHead(node);
        ++numWarmAccesses_;
        setUpdateTime(node, curr);
    }
    return true;

    //return lruMutex_->lock_combine(func);
  }
  return false;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat MM2Q::Container<T, HookPtr>::getEvictionAgeStat(
    uint64_t projectedLength) const noexcept {
    //auto hotlck = LockHolder{*lruMutex_[LruType::Hot], std::defer_lock};
    //auto warmlck = LockHolder{*lruMutex_[LruType::Warm], std::defer_lock};
    //auto warmtaillck = LockHolder{*lruMutex_[LruType::WarmTail], std::defer_lock};
    //auto coldlck = LockHolder{*lruMutex_[LruType::Cold], std::defer_lock};
    //auto coldtaillck = LockHolder{*lruMutex_[LruType::ColdTail], std::defer_lock};
    //std::lock(hotlck,warmlck,warmtaillck,coldlck,coldtaillck);
    std::lock(lruMutex_[LruType::Hot],lruMutex_[LruType::Warm],
              lruMutex_[LruType::WarmTail],lruMutex_[LruType::Cold],
              lruMutex_[LruType::ColdTail]);
    return getEvictionAgeStatLocked(projectedLength);
  //return lruMutex_->lock_combine([this, projectedLength]() {
  //  return getEvictionAgeStatLocked(projectedLength);
  //});
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat MM2Q::Container<T, HookPtr>::getEvictionAgeStatLocked(
    uint64_t projectedLength) const noexcept {
  EvictionAgeStat stat;
  const auto curr = static_cast<Time>(util::getCurrentTimeSec());

  stat.hotQueueStat.oldestElementAge = getOldestAgeLocked(LruType::Hot, curr);
  stat.hotQueueStat.size = lru_[LruType::Hot].size();

  // sum the tail and the main list for the ones that have tail.
  stat.warmQueueStat.oldestElementAge =
      getOldestAgeLocked(LruType::WarmTail, curr);
  stat.warmQueueStat.size = lru_[LruType::Warm].size() +
                            lru_[LruType::WarmTail].size();

  stat.coldQueueStat.oldestElementAge =
      getOldestAgeLocked(LruType::ColdTail, curr);
  stat.coldQueueStat.size = lru_[LruType::ColdTail].size() +
                            lru_[LruType::Cold].size();

  auto it = lru_[LruType::WarmTail].rbegin();
  for (size_t numSeen = 0; numSeen < projectedLength && it != lru_[LruType::WarmTail].rend();
       ++numSeen, ++it) {
  }
  stat.projectedAge = (it != lru_[LruType::WarmTail].rend()) ? curr - getUpdateTime(*it)
                                          : stat.warmQueueStat.oldestElementAge;
  return stat;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
uint32_t MM2Q::Container<T, HookPtr>::getOldestAgeLocked(
    LruType lruType, Time currentTime) const noexcept {
  auto it = lru_[lruType].rbegin();
  return it != lru_[lruType].rend() ? currentTime - getUpdateTime(*it) : 0;
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
//make probablistic and acquire locks here
//
template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::rebalance() noexcept {
  std::scoped_lock lock(lruMutex_[LruType::Hot],
                        lruMutex_[LruType::Warm],
                        lruMutex_[LruType::WarmTail],
                        lruMutex_[LruType::Cold],
                        lruMutex_[LruType::ColdTail]);
  // shrink Warm (and WarmTail) if their total size is larger than expected
  size_t lru_size = lru_[LruType::Hot].size() +
                    lru_[LruType::Warm].size() +
                    lru_[LruType::WarmTail].size() +
                    lru_[LruType::Cold].size() +
                    lru_[LruType::ColdTail].size();

  size_t expectedSize = config_.getWarmSizePercent() * lru_size / 100;
  while (lru_[LruType::Warm].size() +
             lru_[LruType::WarmTail].size() >
         expectedSize) {
    auto popFrom = [&](LruType lruType) -> T* {
      auto& lru = lru_[lruType];
      T* node = lru.getTail();
      XDCHECK(node);
      lru.remove(*node);
      if (lruType == WarmTail || lruType == ColdTail) {
        unmarkTail(*node);
      }
      return node;
    };
    // remove from warm tail if it is not empty. if empty, remove from warm.
    T* node = lru_[LruType::WarmTail].size() > 0
                  ? popFrom(LruType::WarmTail)
                  : popFrom(LruType::Warm);
    XDCHECK(isWarm(*node));
    lru_[LruType::Cold].linkAtHead(*node);
    markCold(*node);
  }

  // shrink Hot if its size is larger than expected
  expectedSize = config_.hotSizePercent * lru_size / 100;
  while (lru_[LruType::Hot].size() > expectedSize) {
    auto node = lru_[LruType::Hot].getTail();
    XDCHECK(node);
    XDCHECK(isHot(*node));
    lru_[LruType::Hot].remove(*node);
    lru_[LruType::Cold].linkAtHead(*node);
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
    std::scoped_lock lock(lruMutex_[LruType::Hot]); 
    if (node.isInMMContainer()) {
      return false;
    }

    markHot(node);
    unmarkCold(node);
    unmarkTail(node);
    lru_[LruType::Hot].linkAtHead(node);
    

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
  std::unique_lock<Mutex> l(lruMutex_[LruType::ColdTail]);
  return Iterator{std::move(l), lru_[LruType::ColdTail].rbegin()};
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
void MM2Q::Container<T, HookPtr>::removeLocked(T& node) noexcept {
  LruType type = getLruType(node);
  lru_[type].remove(node);

  node.unmarkInMMContainer();
  return;
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

    std::scoped_lock lock(lruMutex_[LruType::Hot],
                        lruMutex_[LruType::Warm],
                        lruMutex_[LruType::WarmTail],
                        lruMutex_[LruType::Cold],
                        lruMutex_[LruType::ColdTail]);
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
    std::scoped_lock lock(lruMutex_[LruType::Hot],
                        lruMutex_[LruType::Warm],
                        lruMutex_[LruType::WarmTail],
                        lruMutex_[LruType::Cold],
                        lruMutex_[LruType::ColdTail]);

    return config_;
}

template <typename T, MM2Q::Hook<T> T::*HookPtr>
bool MM2Q::Container<T, HookPtr>::remove(T& node) noexcept {
  auto type = getLruType(node);
  {
    std::scoped_lock lock(lruMutex_[type]); 
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

    
  //return lruMutex_->lock_combine([this, &oldNode, &newNode]() 
    
  auto type1 = getLruType(oldNode);
  auto type2 = getLruType(newNode);
  {
    std::scoped_lock lock(lruMutex_[type1],
                          lruMutex_[type2]);

    if (!oldNode.isInMMContainer() || newNode.isInMMContainer()) {
      return false;
    }
    const auto updateTime = getUpdateTime(oldNode);

    LruType type = getLruType(oldNode);
    lru_[type].replace(oldNode, newNode);
    switch (type) {
    case LruType::Hot:
      markHot(newNode);
      break;
    case LruType::ColdTail:
      markTail(newNode); // pass through to also mark cold
    case LruType::Cold:
      markCold(newNode);
      break;
    case LruType::WarmTail:
      markTail(newNode);
      break; // warm is indicated by not marking hot or cold
    case LruType::Warm:
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
void MM2Q::Container<T, HookPtr>::adjustTail(LruType list) {
  LruType tail;
  if (list == LruType::Warm)
      tail = LruType::WarmTail;
  else if (list == LruType::Cold) 
      tail = LruType::ColdTail;
  auto ptr = lru_[tail].getTail();
  while (ptr && lru_[tail].size() + 1 <= config_.tailSize) {
    markTail(*ptr);
    lru_[list].remove(*ptr);
    lru_[tail].linkAtHead(*ptr);
    ptr = lru_[list].getTail();
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
