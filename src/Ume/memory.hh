/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
\file Ume/memory.hh
*/

#ifndef UME_MEMORY_HH
#define UME_MEMORY_HH

#include "process_mgmt.hh"
#include "utils.hh"
#include <Kokkos_Core.hpp>
#include <type_traits>

/* Define the default memory space for scratch arrays */
#if defined(KOKKOS_ENABLE_CUDA)
using DefaultMemSpace = Kokkos::SharedSpace;
#elif defined(KOKKOS_ENABLE_HIP)
using DefaultMemSpace = Kokkos::HIPSpace;
#else
using DefaultMemSpace = Kokkos::SharedSpace;
#endif

/* Define the default execution spaces */
using DevExecSpace = Kokkos::DefaultExecutionSpace;
using HostExecSpace = Kokkos::DefaultHostExecutionSpace;

/* NOTE: We use left layout to avoid transposing arrays across
 * Fortran/C++ language barriers. For maximum performance, we
 * should be using right layout but the size of the "bad" dimension
 * is usuallly small. */
using MemLayout = Kokkos::LayoutLeft;

/* Trait to prevent view from being memory managed by Kokkos */
using MemUnmanaged = Kokkos::MemoryTraits<Kokkos::Unmanaged>;

namespace MemOpts {
struct DoNotCopyInit : std::false_type {};
struct CopyInit : std::true_type {};
} // namespace MemOpts

template <typename MemorySpace> class MemoryPoolAllocation {
private:
  struct BlockClaim {
    /* This represents a particular claim of some number of blocks of memory
     * from the allocated pool -- block offset in pool and number of blocks in
     * this claim. */
    size_t start, length;
  };

  /* To avoid our vector of BlockClaims from resizing, let's reserve
   * space for a set number of them. */
  static constexpr unsigned num_claims_reservations = 32;

  using AllocFunc = std::function<void *(size_t const)>;
  using FreeFunc = std::function<void(void *)>;

  AllocFunc Alloc_;
  FreeFunc Free_;
  void *pool_;
  size_t num_blocks_;
  unsigned block_size_;
  unsigned block_size_in_size_t_; // for pointer arithmetic
  std::vector<BlockClaim> claims_;

  void *GetPtrToOffsetInPool(size_t const block_index) const {
    static_assert(sizeof(size_t) == sizeof(void *) &&
            sizeof(void *) == sizeof(unsigned long),
        "Unsupported pointer to integer conversion on this platform");
    return static_cast<void *>(
        static_cast<size_t *>(pool_) + block_index * block_size_in_size_t_);
  }

public:
  size_t SizeInBytes() const { return num_blocks_ * block_size_; }
  void Nullify() {
    pool_ = nullptr;
    num_blocks_ = 0;
    block_size_ = 0;
    block_size_in_size_t_ = 0;
    claims_ = std::vector<BlockClaim>();
  };
  MemoryPoolAllocation() { Nullify(); };

  // Default (de)allocation through Kokkos
  MemoryPoolAllocation(
      size_t const num_bytes, unsigned const block_size,
      AllocFunc alloc = [](size_t const num_bytes) -> void * {
        return Kokkos::kokkos_malloc<MemorySpace>("Pool allocation", num_bytes);
      },
      FreeFunc free = [](void *p) -> void {
        Kokkos::kokkos_free<MemorySpace>(p);
      }) {
    Nullify();

    /* It would be nice to have error checking on block_size > 0, num_bytes > 0,
     * block_size a multiple of sizeof(size_t), etc. */
    block_size_ = block_size;
    block_size_in_size_t_ = block_size / sizeof(size_t);
    num_blocks_ = (num_bytes - 1) / block_size + 1;
    Alloc_ = alloc;
    Free_ = free;
    claims_ = std::vector<BlockClaim>();
    claims_.reserve(num_claims_reservations);
    pool_ = Alloc_(num_blocks_ * block_size);

    if (!pool_) {
      printf("MemoryPoolAllocation: insufficient free memory to allocate\n"
             "  %zu bytes for %zu blocks of size %d\n",
          num_bytes, num_blocks_, block_size_);
      Ume::error_stop("MemoryPoolAllocation failed");
    }
  } // MemoryPoolAllocation

  /* Only allow moving, NOT copying */
  MemoryPoolAllocation(MemoryPoolAllocation &&rhs) {
    pool_ = rhs.pool_;
    Alloc_ = rhs.Alloc_;
    Free_ = rhs.Free_;
    num_blocks_ = rhs.num_blocks_;
    block_size_ = rhs.block_size_;
    block_size_in_size_t_ = rhs.block_size_in_size_t_;
    claims_ = rhs.claims_;
    rhs.Nullify();
  }
  MemoryPoolAllocation &operator=(MemoryPoolAllocation &&rhs) {
    if (this == &rhs)
      return *this;

    Finalize();

    pool_ = rhs.pool_;
    Alloc_ = rhs.Alloc_;
    Free_ = rhs.Free_;
    num_blocks_ = rhs.num_blocks_;
    block_size_ = rhs.block_size_;
    block_size_in_size_t_ = rhs.block_size_in_size_t_;
    claims_ = rhs.claims_;
    rhs.Nullify();
    return *this;
  }
  MemoryPoolAllocation(MemoryPoolAllocation const &) = delete;
  MemoryPoolAllocation &operator=(MemoryPoolAllocation const &) = delete;

  ~MemoryPoolAllocation() { Finalize(); }

  void Finalize() {
    assert(claims_.empty());

    if (pool_)
      Free_(pool_);

    Nullify();
  }

  /* Claim some number of blocks from the already allocated pool.
   * Returns pointer to location within the pool. */
  void *Claim(size_t const num_bytes) {
    // This is a valid usecase, but we need not get anything from the pool
    if (num_bytes == 0)
      return nullptr;

    size_t const num_blocks_needed = (num_bytes - 1) / block_size_ + 1;
    // If we don't have any claims, just use the start of the pool
    if (claims_.empty()) {
      if (num_blocks_ >= num_blocks_needed) {
        claims_.push_back(BlockClaim(0, num_blocks_needed));
        return pool_;
      }
    } else {
      // Try space before the first claim
      if (claims_.front().start >= num_blocks_needed) {
        claims_.insert(claims_.begin(), BlockClaim(0, num_blocks_needed));
        return pool_;
      }
      // Try space after last claim
      {
        auto pos_of_next_claim = claims_.back().start + claims_.back().length;
        if (num_blocks_ - pos_of_next_claim > num_blocks_needed) {
          claims_.push_back(BlockClaim(pos_of_next_claim, num_blocks_needed));
          return GetPtrToOffsetInPool(pos_of_next_claim);
        }
      }
      // Try space between claims

      for (auto i = 0; i < static_cast<int>(claims_.size()) - 1; ++i) {
        auto this_claim_end = claims_[i].start + claims_[i].length;
        if (claims_[i + 1].start - this_claim_end > num_blocks_needed) {
          claims_.insert(claims_.begin() + i + 1,
              BlockClaim(this_claim_end, num_blocks_needed));
          return GetPtrToOffsetInPool(claims_[i + 1].start);
        }
      }
    }

    /* Claim doesn't fit anywhere, so we can't allocate it */
    Ume::error_stop("Memory pool cannot claim memory.");
    return nullptr;
  } // Claim

  /* Release some number of blocks back to the pool.
   * Returns number of bytes released.
   * This does not free any memory. */
  size_t Release(void *p) {
    for (int i = static_cast<int>(claims_.size()) - 1; i >= 0; ++i) {
      if (p == GetPtrToOffsetInPool(claims_[i].start)) {
        size_t const bytes = claims_[i].length * block_size_;
        claims_.erase(claims_.begin() + i);
        return bytes;
      }
    }
    return 0;
  } // Release

}; // class MemoryPoolAllocation

template <typename ViewDataType, typename MemorySpace> class LifetimeWrapper;

template <typename MemorySpace> class MemoryPool {
private:
  MemoryPool() {
    is_initialized_ = false;
    pool_enabled_ = false;
  }
  ~MemoryPool() = default;

  MemoryPoolAllocation<MemorySpace> pool_;
  bool is_initialized_;
  bool pool_enabled_;

public:
  MemoryPool(const MemoryPool &) = delete;
  MemoryPool &operator=(const MemoryPool &) = delete;

  static MemoryPool &GetInstance() {
    static MemoryPool instance;
    return instance;
  }

  bool &IsInitialized() { return is_initialized_; }
  bool &IsPoolEnabled() { return pool_enabled_; }
  auto &Pool() { return pool_; }

  template <typename T, typename CopyOpt, typename... Dims>
  auto GetViewImpl(T const &init_value, CopyOpt &&opt, Dims &&...dims) {
    /* Separate dims and memopt, only forward dims */
    using ReturnType = LifetimeWrapper<
        typename Ume::detail::AddPointerNTimes<T, sizeof...(Dims)>::type,
        MemorySpace>;
    size_t const num_entries = (1 * ... * dims);
    size_t const num_bytes = num_entries * sizeof(T);

    return ReturnType{static_cast<T *>(pool_.Claim(num_bytes)), init_value,
        std::forward<CopyOpt>(opt), std::forward<Dims>(dims)...};
  }

  template <typename T, typename CopyOpt, typename Tuple, std::size_t... Is>
  auto GetViewImplTuple(T const &init_value, CopyOpt &&opt, Tuple &&t,
      std::index_sequence<Is...>) {
    return GetViewImpl(init_value, std::forward<CopyOpt>(opt),
        std::get<Is>(std::forward<Tuple>(t))...);
  }

  /* Check final parameter in pack, If not a copy option, append copy option. */
  template <typename T, typename... Args>
  auto GetView(T const &init_value, Args &&...args) {
    /* The args in this case can either be a sequence of dimensions followed by
     * a copy option or just a sequence of dimensions. This function will add
     * the default copy option if one wasn't provided.
     * -----
     * args... = dims... , copy_option
     * - or -
     * args... = dims...
     * ------
     * - if -
     * args... = dims..., copy_option
     * - then -
     * call getview(init_value, copy_option, dims...)
     * - else (no copy option provided and args... = dims...) -
     * Note: CopyInit{} will produce the default behavior of copy initializing
     * the view call getview(init_value, CopyInit{}, args...) (again, args... =
     * dims...)
     * -----
     * size (number of arguments) of parameter pack */
    constexpr auto N = sizeof...(Args);

    if constexpr (N == 0) { // No copy arg passed and no other arguments either
      return GetViewImpl(
          init_value, MemOpts::CopyInit{}, std::forward<Args>(args)...);
    } else { // Parameter pack as tuple
      auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);

      /* Get underlying type of last arg of tuple */
      using LastUnclean = std::tuple_element_t<N - 1, decltype(tuple)>;
      using Last_t = typename std::remove_cv_t<
          typename std::remove_reference<LastUnclean>::type>;

      /* If that type is one of the copy options, pass in the copy option and
       * separate out the dims as a new pack. */
      if constexpr (std::is_same_v<Last_t, MemOpts::CopyInit> ||
          std::is_same_v<Last_t, MemOpts::DoNotCopyInit>) {
        /* Call the tuple version to create a new parameter pack that doesn't
         * include the last argument. */
        return GetViewImplTuple(
            init_value, Last_t{}, tuple, std::make_index_sequence<N - 1>{});
      } else {
        /* Dims were passed-in but no copy opt so use the CopyInit as the
         * default. */
        return GetViewImpl(
            init_value, MemOpts::CopyInit{}, std::forward<Args>(args)...);
      }
    }
  }
}; // class MemoryPool

template <typename ViewDataType, typename MemorySpace>
class LifetimeWrapper
    : public Kokkos::View<ViewDataType, MemLayout, MemorySpace, MemUnmanaged> {
  using Base = Kokkos::View<ViewDataType, MemLayout, MemorySpace, MemUnmanaged>;
  using ValueType = Base::value_type;

  unsigned int *ref_count_ = nullptr;

  KOKKOS_INLINE_FUNCTION
  bool RefCounted() const { return ref_count_ != nullptr; }

  void DecrementRefCounterAndMaybeRelease() {
    if (RefCounted()) {
      // TODO: theadsafety
      --(*(ref_count_));
      if (0 == *ref_count_) {
        MemoryPool<DefaultMemSpace>::GetInstance().Pool().Release(this->data());
        delete ref_count_;
        ref_count_ = nullptr;
      }
    }
  }

public:
  // LifetimeWrapper() = delete;

  template <typename PtrType, typename CopyOpt, typename... Dims>
  LifetimeWrapper(PtrType p, ValueType const &v, [[maybe_unused]] CopyOpt &&opt,
      Dims &&...dims)
      : Base{p, static_cast<size_t>(dims)...}, ref_count_(nullptr) {
    /* NOTE: there are valid usecases where p is a nullptr, but we need not
     * refcount those. */
    if (p != nullptr) {
      /* The copy options inherit from true type and false type as appropriate
       * thus this contexpr if expression will do the correct thing by not doing
       * the deep copy if DoNotCopyInit is passed in. */
      if constexpr (CopyOpt::value) {
        Kokkos::deep_copy(DevExecSpace(), *this, v);
        DevExecSpace().fence();
      }

      // TODO: threadsafety
      ref_count_ = new unsigned int;
      *(ref_count_) = 1;
    }
  }

  // Copying
  KOKKOS_INLINE_FUNCTION
  LifetimeWrapper(LifetimeWrapper const &rhs) : Base(rhs) {
    this->ref_count_ = rhs.ref_count_;
    if (RefCounted()) {
      KOKKOS_IF_ON_HOST(
          // TODO: threadsafety
          ++(*(this->ref_count_));)
    }
  }
  KOKKOS_INLINE_FUNCTION
  LifetimeWrapper &operator=(LifetimeWrapper const &rhs) {
    KOKKOS_IF_ON_HOST(this->DecrementRefCounterAndMaybeRelease();)
    Base::operator=(rhs);
    this->ref_count_ = rhs.ref_count_;
    if (RefCounted()) {
      KOKKOS_IF_ON_HOST(
          // TODO: threadsafety
          ++(*(this->ref_count_));)
    }
    return *this;
  }

  // Moving
  KOKKOS_INLINE_FUNCTION
  LifetimeWrapper(LifetimeWrapper &&rhs) noexcept : Base(std::move(rhs)) {
    this->ref_count_ = rhs.ref_count_;
    rhs.ref_count_ = nullptr;
  }
  KOKKOS_INLINE_FUNCTION
  LifetimeWrapper &operator=(LifetimeWrapper &&rhs) noexcept {
    KOKKOS_IF_ON_HOST(this->DecrementRefCounterAndMaybeRelease();)
    Base::operator=(std::move(rhs));
    this->ref_count_ = rhs.ref_count_;
    rhs.ref_count_ = nullptr;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION
  ~LifetimeWrapper() {
    KOKKOS_IF_ON_HOST(DecrementRefCounterAndMaybeRelease();)
  }
}; // class LifetimeWrapper

/* Return a reference to the memory pool. */
inline MemoryPool<DefaultMemSpace> &GetMemPool() {
  return MemoryPool<DefaultMemSpace>::GetInstance();
}

#endif
