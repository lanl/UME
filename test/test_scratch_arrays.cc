/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

#include "Ume/VecN.hh"
#include "Ume/array_types.hh"
#include "Ume/memory.hh"
#include <catch2/catch_test_macros.hpp>
#include <concepts>

using HostExecMemSpace = Kokkos::DefaultHostExecutionSpace::memory_space;

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
      "assign 1D int scratch array", dim0,
      KOKKOS_LAMBDA(const int i) { scratch_array(i) = i; });

#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_HIP)
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

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
      "assign 2D int scratch array", dim1, KOKKOS_LAMBDA(const int j) {
        for (int i = 0; i < dim0; ++i)
          scratch_array(j, i) = j * i;
      });

#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_HIP)
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

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
      "assign 1D double scratch array", dim0, KOKKOS_LAMBDA(const int i) {
        scratch_array(i) = static_cast<double>(i);
      });

#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_HIP)
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

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
      "assign 2D double scratch array", dim1, KOKKOS_LAMBDA(const int j) {
        for (int i = 0; i < dim0; ++i)
          scratch_array(j, i) = static_cast<double>(j * i);
      });

#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_HIP)
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

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
      "assign 1D Vec3 scratch array", dim0,
      KOKKOS_LAMBDA(const int i) { scratch_array(i) = Ume::Vec3(static_cast<double>(i)); });

#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_HIP)
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

  REQUIRE(host_scratch_array(0) == Ume::Vec3(0));
  REQUIRE(host_scratch_array(dim0 - 1) == Ume::Vec3(dim0 - 1));
}
