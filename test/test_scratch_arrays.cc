/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

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
      KOKKOS_LAMBDA(int const i) { scratch_array(i) = i; });

#ifdef KOKKOS_ENABLE_CUDA
  Kokkos::fence();
  Kokkos::deep_copy(host_scratch_array, scratch_array);
#endif

  REQUIRE(host_scratch_array(0) == 0);
  REQUIRE(host_scratch_array(dim0 - 1) == dim0 - 1);
}
