/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

#include "Ume/DS_Types.hh"
#include "Ume/VecN.hh"
#include "Ume/array_types.hh"
#include "Ume/mem_exec_spaces.hh"
#include "Ume/memory.hh"
#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <cstddef>
#include <vector>

TEST_CASE("1D int scratch array"
          "[ArrayRank1<int>]") {
  constexpr int dim0 = 512;
  auto scratch_array = NewArrayRank1<int>("int scratch array", dim0);
  auto host_scratch_array =
      Kokkos::create_mirror_view(HostExecMemSpace(), scratch_array);

  static_assert(std::same_as<decltype(scratch_array),
                    LifetimeWrapper<int *, DefaultMemSpace>>,
      "Variable must be a reference-counted Rank-1 integer array type.");
  static_assert(
      std::same_as<decltype(static_cast<ArrayRank1<int>>(scratch_array)),
          ArrayRank1<int>>,
      "Variable must be castable to a Kokkos view.");

  Kokkos::parallel_for(
      "assign 1D int scratch array", Kokkos::RangePolicy<DevExecSpace>(0, dim0),
      KOKKOS_LAMBDA(const int i) { scratch_array(i) = i; });

  Kokkos::deep_copy(host_scratch_array, scratch_array);

  REQUIRE(host_scratch_array(0) == 0);
  REQUIRE(host_scratch_array(dim0 - 1) == dim0 - 1);
}

TEST_CASE("2D int scratch array"
          "[ArrayRank2<int>]") {
  constexpr int dim0 = 3;
  constexpr int dim1 = 512;
  auto scratch_array = NewArrayRank2<int>("int scratch array", dim1, dim0);
  auto host_scratch_array =
      Kokkos::create_mirror_view(HostExecMemSpace(), scratch_array);

  static_assert(std::same_as<decltype(scratch_array),
                    LifetimeWrapper<int **, DefaultMemSpace>>,
      "Variable must be a reference-counted Rank-1 integer array type.");
  static_assert(
      std::same_as<decltype(static_cast<ArrayRank2<int>>(scratch_array)),
          ArrayRank2<int>>,
      "Variable must be castable to a Kokkos view.");

  Kokkos::parallel_for(
      "assign 2D int scratch array", Kokkos::RangePolicy<DevExecSpace>(0, dim1),
      KOKKOS_LAMBDA(const int j) {
        for (int i = 0; i < dim0; ++i)
          scratch_array(j, i) = j * i;
      });

  Kokkos::deep_copy(host_scratch_array, scratch_array);

  REQUIRE(host_scratch_array(0, 0) == 0);
  REQUIRE(host_scratch_array(0, dim0 - 1) == 0);
  REQUIRE(host_scratch_array(dim1 - 1, 0) == 0);
  REQUIRE(host_scratch_array(1, dim0 - 1) == dim0 - 1);
  REQUIRE(host_scratch_array(dim1 - 1, 1) == dim1 - 1);
  REQUIRE(host_scratch_array(dim1 - 1, dim0 - 1) == (dim1 - 1) * (dim0 - 1));
}

TEST_CASE("1D double scratch array"
          "[ArrayRank1<double>]") {
  constexpr int dim0 = 512;
  auto scratch_array = NewArrayRank1<double>("double scratch array", dim0);
  auto host_scratch_array =
      Kokkos::create_mirror_view(HostExecMemSpace(), scratch_array);

  static_assert(std::same_as<decltype(scratch_array),
                    LifetimeWrapper<double *, DefaultMemSpace>>,
      "Variable must be a reference-counted Rank-1 double array type.");
  static_assert(
      std::same_as<decltype(static_cast<ArrayRank1<double>>(scratch_array)),
          ArrayRank1<double>>,
      "Variable must be castable to a Kokkos view.");

  Kokkos::parallel_for(
      "assign 1D double scratch array",
      Kokkos::RangePolicy<DevExecSpace>(0, dim0), KOKKOS_LAMBDA(const int i) {
        scratch_array(i) = static_cast<double>(i);
      });

  Kokkos::deep_copy(host_scratch_array, scratch_array);

  REQUIRE(host_scratch_array(0) == static_cast<double>(0));
  REQUIRE(host_scratch_array(dim0 - 1) == static_cast<double>(dim0 - 1));
}

TEST_CASE("2D double scratch array"
          "[ArrayRank2<double>]") {
  constexpr int dim0 = 3;
  constexpr int dim1 = 512;
  auto scratch_array =
      NewArrayRank2<double>("double scratch array", dim1, dim0);
  auto host_scratch_array =
      Kokkos::create_mirror_view(HostExecMemSpace(), scratch_array);

  static_assert(std::same_as<decltype(scratch_array),
                    LifetimeWrapper<double **, DefaultMemSpace>>,
      "Variable must be a reference-counted Rank-1 double array type.");
  static_assert(
      std::same_as<decltype(static_cast<ArrayRank2<double>>(scratch_array)),
          ArrayRank2<double>>,
      "Variable must be castable to a Kokkos view.");

  Kokkos::parallel_for(
      "assign 2D double scratch array",
      Kokkos::RangePolicy<DevExecSpace>(0, dim1), KOKKOS_LAMBDA(const int j) {
        for (int i = 0; i < dim0; ++i)
          scratch_array(j, i) = static_cast<double>(j * i);
      });

  Kokkos::deep_copy(host_scratch_array, scratch_array);

  REQUIRE(host_scratch_array(0, 0) == static_cast<double>(0));
  REQUIRE(host_scratch_array(0, dim0 - 1) == static_cast<double>(0));
  REQUIRE(host_scratch_array(dim1 - 1, 0) == static_cast<double>(0));
  REQUIRE(host_scratch_array(1, dim0 - 1) == static_cast<double>(dim0 - 1));
  REQUIRE(host_scratch_array(dim1 - 1, 1) == static_cast<double>(dim1 - 1));
  REQUIRE(host_scratch_array(dim1 - 1, dim0 - 1) ==
      static_cast<double>((dim1 - 1) * (dim0 - 1)));
}

TEST_CASE("1D Vec3 scratch array"
          "[ArrayRank1<Vec3>]") {
  constexpr int dim0 = 512;
  auto scratch_array = NewArrayRank1<Ume::Vec3>(
      "Vec3 scratch array", dim0, Ume::Vec3(777777777.0));
  auto host_scratch_array =
      Kokkos::create_mirror_view(HostExecMemSpace(), scratch_array);

  static_assert(std::same_as<decltype(scratch_array),
                    LifetimeWrapper<Ume::Vec3 *, DefaultMemSpace>>,
      "Variable must be a reference-counted Rank-1 Vec3 array type.");
  static_assert(
      std::same_as<decltype(static_cast<ArrayRank1<Ume::Vec3>>(scratch_array)),
          ArrayRank1<Ume::Vec3>>,
      "Variable must be castable to a Kokkos view.");

  Kokkos::parallel_for(
      "assign 1D Vec3 scratch array",
      Kokkos::RangePolicy<DevExecSpace>(0, dim0), KOKKOS_LAMBDA(const int i) {
        scratch_array(i) = Ume::Vec3(static_cast<double>(i));
      });

  Kokkos::deep_copy(host_scratch_array, scratch_array);

  REQUIRE(host_scratch_array(0) == Ume::Vec3(0));
  REQUIRE(host_scratch_array(dim0 - 1) == Ume::Vec3(dim0 - 1));
}

TEST_CASE("STL vector view copy/set/copy-back"
          "[DBLV_T]") {
  constexpr int dim0 = 512;

  Ume::DS_Types::DBLV_T var(dim0, 0.0);
  Kokkos::View<double *, HostSpace> host_var(var.data(), var.size());
  auto device_var =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), host_var);

  Ume::DS_Types::DBLV_T const const_var(dim0, 777777777.0);
  Kokkos::View<double const *, HostSpace> host_const_var(
      const_var.data(), const_var.size());
  auto const device_const_var =
      Kokkos::create_mirror_view_and_copy(DevExecMemSpace(), host_const_var);

  Kokkos::parallel_for(
      "assign to STL vector", Kokkos::RangePolicy<DevExecSpace>(0, dim0),
      KOKKOS_LAMBDA(const int i) { device_var(i) = device_const_var(i); });

  Kokkos::deep_copy(host_var, device_var);

  REQUIRE(host_var(0) == host_const_var(0));
  REQUIRE(var[0] == const_var[0]);
  REQUIRE(host_var(dim0 - 1) == host_const_var(dim0 - 1));
  REQUIRE(var[dim0 - 1] == const_var[dim0 - 1]);
}

namespace {

/* A MemoryPoolAllocation over a host buffer the test owns, so the pool's
   offsets can be checked as plain pointer arithmetic on any backend. */
class TestPool {
public:
  static constexpr unsigned block_size = 64;

  explicit TestPool(std::size_t const num_blocks)
      : storage_(num_blocks * block_size / sizeof(std::size_t)),
        pool_(
            num_blocks * block_size, block_size,
            [this](std::size_t const) -> void * { return storage_.data(); },
            [](void *) {}) {}

  MemoryPoolAllocation<HostSpace> &operator*() { return pool_; }
  MemoryPoolAllocation<HostSpace> *operator->() { return &pool_; }

  /* Address of block `b` of the pool. */
  void *block(std::size_t const b) {
    return reinterpret_cast<std::byte *>(storage_.data()) + b * block_size;
  }

private:
  std::vector<std::size_t> storage_;
  MemoryPoolAllocation<HostSpace> pool_;
};

} // namespace

TEST_CASE("MemoryPoolAllocation releases a claim that is not the last",
    "[MemoryPoolAllocation]") {
  constexpr auto bs = TestPool::block_size;
  TestPool pool(4);

  void *const a = pool->Claim(bs);
  void *const b = pool->Claim(bs);
  void *const c = pool->Claim(bs);
  REQUIRE(a == pool.block(0));
  REQUIRE(b == pool.block(1));
  REQUIRE(c == pool.block(2));

  // a is the first of three claims; Release walks claims_ from the back.
  CHECK(pool->Release(a) == bs);
  // Positive control: a is no longer claimed, so releasing it again finds
  // nothing.
  CHECK(pool->Release(a) == 0);

  // The freed block before the first claim is reused.
  void *const d = pool->Claim(bs);
  CHECK(d == pool.block(0));

  CHECK(pool->Release(b) == bs);
  CHECK(pool->Release(c) == bs);
  CHECK(pool->Release(d) == bs);
}

TEST_CASE("MemoryPoolAllocation claims an exact fit after the last claim",
    "[MemoryPoolAllocation]") {
  constexpr auto bs = TestPool::block_size;
  TestPool pool(4);

  void *const a = pool->Claim(3 * bs);
  REQUIRE(a == pool.block(0));

  // One block remains after a, and one block is asked for.
  void *const b = pool->Claim(bs);
  CHECK(b == pool.block(3));

  CHECK(pool->Release(b) == bs);
  CHECK(pool->Release(a) == 3 * bs);
}

TEST_CASE("MemoryPoolAllocation claims an exact fit between claims",
    "[MemoryPoolAllocation]") {
  constexpr auto bs = TestPool::block_size;
  TestPool pool(5);

  void *const a = pool->Claim(bs);
  void *const b = pool->Claim(2 * bs);
  void *const c = pool->Claim(bs);
  REQUIRE(a == pool.block(0));
  REQUIRE(b == pool.block(1));
  REQUIRE(c == pool.block(3));

  // Leaves a two-block gap at [1, 3) and one block after c, so a two-block
  // claim fits only in the gap, exactly.
  CHECK(pool->Release(b) == 2 * bs);
  void *const d = pool->Claim(2 * bs);
  CHECK(d == pool.block(1));

  CHECK(pool->Release(a) == bs);
  CHECK(pool->Release(c) == bs);
  CHECK(pool->Release(d) == 2 * bs);
}
