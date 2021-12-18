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

#include <folly/logging/xlog.h>

#include <memory>

#include "cachelib/allocator/memory/Slab.h"

namespace facebook {
namespace cachelib {

class SlabAllocator;

template <typename PtrType, typename AllocatorContainer>
class PtrCompressor;

// the following are for pointer compression for the memory allocator.  We
// compress pointers by storing the slab index and the alloc index of the
// allocation inside the slab. With slab worth kNumSlabBits of data, if we
// have the min allocation size as 64 bytes, that requires kNumSlabBits - 6
// bits for storing the alloc index. This leaves the remaining (32 -
// (kNumSlabBits - 6)) bits for the slab index.  Hence we can index 256 GiB
// of memory in slabs and index anything more than 64 byte allocations inside
// the slab using a 32 bit representation.
//
// This CompressedPtr makes decompression fast by staying away from division and
// modulo arithmetic and doing those during the compression time. We most often
// decompress a CompressedPtr than compress a pointer while creating one.
class CACHELIB_PACKED_ATTR CompressedPtr {
 public:
  using PtrType = uint64_t;
  // Thrift doesn't support unsigned type
  using SerializedPtrType = int64_t;

  // Total number of bits to represent a CompressPtr.
  static constexpr size_t kNumBits = NumBits<PtrType>::value;

  // true if the compressed ptr expands to nullptr.
  bool isNull() const noexcept { return ptr_ == kNull; }

  bool operator==(const PtrType ptr) const noexcept { return ptr_ == ptr; }
  bool operator==(const CompressedPtr ptr) const noexcept {
    return ptr_ == ptr.ptr_;
  }
  bool operator!=(const PtrType ptr) const noexcept { return !(ptr == ptr_); }
  // If the allocSize is smaller than this, then pointer compression is not
  // going to work.
  static constexpr uint32_t getMinAllocSize() noexcept {
    return static_cast<uint32_t>(1) << (Slab::kMinAllocPower);
  }

  // maximum adressable memory for pointer compression to work.
  static constexpr size_t getMaxAddressableSize() noexcept {
    return static_cast<size_t>(1) << (kNumSlabIdxBits + Slab::kNumSlabBits);
  }

  // default construct to nullptr.
  CompressedPtr() = default;

  // Restore from serialization
  explicit CompressedPtr(SerializedPtrType ptr)
      : ptr_(static_cast<PtrType>(ptr)) {}

  SerializedPtrType saveState() const noexcept {
    return static_cast<SerializedPtrType>(ptr_);
  }

  PtrType getRaw() const noexcept { return ptr_; }

 private:
  // null pointer representation. This is almost never guaranteed to be a
  // valid pointer that we can compress to.
  static constexpr PtrType kNull = 0x00000000ffffffff;

  // default construct to null.
  PtrType ptr_{kNull};

  // create a compressed pointer for a valid memory allocation.
  CompressedPtr(uint32_t slabIdx, uint32_t allocIdx, TierId tid = 0)
      : ptr_(compress(slabIdx, allocIdx, tid)) {}

  constexpr explicit CompressedPtr(PtrType ptr) noexcept : ptr_{ptr} {}

  // number of bits for the Allocation offset in a slab.  With slab size of 22
  // bits and minimum allocation size of 64 bytes, this will be the bottom 16
  // bits of the compressed ptr.
  static constexpr unsigned int kNumAllocIdxBits =
      Slab::kNumSlabBits - Slab::kMinAllocPower;

  // Use topmost 32 bits for TierId
  // XXX: optimize
  static constexpr unsigned int kNumTierIdxOffset = 32;

  static constexpr PtrType kAllocIdxMask = ((PtrType)1 << kNumAllocIdxBits) - 1;

  // kNumTierIdxBits most significant bits
  static constexpr PtrType kTierIdxMask = (((PtrType)1 << kNumTierIdxOffset) - 1) << (NumBits<PtrType>::value - kNumTierIdxOffset);

  // Number of bits for the slab index. This will be the top 16 bits of the
  // compressed ptr.
  static constexpr unsigned int kNumSlabIdxBits =
      NumBits<PtrType>::value - kNumTierIdxOffset - kNumAllocIdxBits; 

  // Compress the given slabIdx and allocIdx into a 64-bit compressed
  // pointer.
  static PtrType compress(uint32_t slabIdx, uint32_t allocIdx, TierId tid) noexcept {
    XDCHECK_LE(allocIdx, kAllocIdxMask);
    XDCHECK_LT(slabIdx, (1u << kNumSlabIdxBits) - 1);
    return (static_cast<uint64_t>(tid) << kNumTierIdxOffset) + (slabIdx << kNumAllocIdxBits) + allocIdx;
  }

  // Get the slab index of the compressed ptr
  uint32_t getSlabIdx() const noexcept {
    XDCHECK(!isNull());
    auto noTierIdPtr = ptr_ & ~kTierIdxMask;
    return static_cast<uint32_t>(noTierIdPtr >> kNumAllocIdxBits);
  }

  // Get the allocation index of the compressed ptr
  uint32_t getAllocIdx() const noexcept {
    XDCHECK(!isNull());
    auto noTierIdPtr = ptr_ & ~kTierIdxMask;
    return static_cast<uint32_t>(noTierIdPtr & kAllocIdxMask);
  }

  uint32_t getTierId() const noexcept {
    XDCHECK(!isNull());
    return static_cast<uint32_t>(ptr_ >> kNumTierIdxOffset);
  }

  void setTierId(TierId tid) noexcept {
    ptr_ += static_cast<uint64_t>(tid) << kNumTierIdxOffset;
  }

  friend SlabAllocator;
  template <typename CPtrType, typename AllocatorContainer>
  friend class PtrCompressor;
};

template <typename PtrType, typename AllocatorT>
class SingleTierPtrCompressor {
 public:
  explicit SingleTierPtrCompressor(const AllocatorT& allocator) noexcept
      : allocator_(allocator) {}

  const CompressedPtr compress(const PtrType* uncompressed) const {
    return allocator_.compress(uncompressed);
  }

  PtrType* unCompress(const CompressedPtr compressed) const {
    return static_cast<PtrType*>(allocator_.unCompress(compressed));
  }

  bool operator==(const SingleTierPtrCompressor& rhs) const noexcept {
    return &allocator_ == &rhs.allocator_;
  }

  bool operator!=(const SingleTierPtrCompressor& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  // memory allocator that does the pointer compression.
  const AllocatorT& allocator_;
};

template <typename PtrType, typename AllocatorContainer>
class PtrCompressor {
 public:
  explicit PtrCompressor(const AllocatorContainer& allocators) noexcept
      : allocators_(allocators) {}

  const CompressedPtr compress(const PtrType* uncompressed) const {
    if (uncompressed == nullptr)
      return CompressedPtr{};

    TierId tid;
    for (tid = 0; tid < allocators_.size(); tid++) {
      if (allocators_[tid]->isMemoryInAllocator(static_cast<const void*>(uncompressed)))
        break;
    }

    auto cptr = allocators_[tid]->compress(uncompressed);
    cptr.setTierId(tid);

    return cptr;
  }

  PtrType* unCompress(const CompressedPtr compressed) const {
    if (compressed.isNull()) {
      return nullptr;
    }

    auto &allocator = *allocators_[compressed.getTierId()];
    return static_cast<PtrType*>(allocator.unCompress(compressed));
  }

  bool operator==(const PtrCompressor& rhs) const noexcept {
    return &allocators_ == &rhs.allocators_;
  }

  bool operator!=(const PtrCompressor& rhs) const noexcept {
    return !(*this == rhs);
  }

 private:
  // memory allocator that does the pointer compression.
  const AllocatorContainer& allocators_;
};
} // namespace cachelib
} // namespace facebook
