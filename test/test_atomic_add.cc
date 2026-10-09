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
  \file test_atomic_add.cc

  Tests for Ume::ume_atomic_add, the accumulation the mesh kernels use to add
  into an element that several iterations of a parallel loop share.
*/

#include "Ume/VecN.hh"
#include "Ume/atomic_add.hh"
#include "Ume/mem_exec_spaces.hh"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

namespace {

/* The number of entries of `got` that differ from `want`. */
template <class T>
std::size_t count_wrong(std::vector<T> const &got, std::vector<T> const &want) {
  std::size_t wrong = 0;
  for (std::size_t i = 0; i < want.size(); ++i)
    if (!(got[i] == want[i]))
      ++wrong;
  return wrong;
}

/* What iteration i adds.  The components are small integers, so every partial
   sum is exact and the totals do not depend on the order of the adds; they
   differ from one another, so a component added into the wrong slot shows. */
Ume::Vec3 term(int const i) {
  return Ume::Vec3(std::array{static_cast<double>(i % 3),
      static_cast<double>(i % 5), -static_cast<double>(i % 7)});
}

} // namespace

/* Every iteration of a host parallel loop adds into one of a few bins, chosen
   round-robin so that every thread's block of the loop hits every bin, and
   each bin must end up with every add: an int count, a double sum, a Vec3 sum,
   and a Vec3 difference taken as the sum of the negations, which is how
   VAR_corner_csurf subtracts.  The loop is repeated because a lost update need
   not show on any one run. */
TEST_CASE(
    "ume_atomic_add keeps every add, whichever thread makes it", "[atomic]") {
  constexpr int nbin = 4;
  constexpr int nadd = 1 << 20;
  constexpr int repetitions = 10;
  INFO("host concurrency " << HostExecSpace().concurrency());

  std::vector<int> want_count(nbin, 0);
  std::vector<double> want_sum(nbin, 0.0);
  std::vector<Ume::Vec3> want_vsum(nbin, Ume::Vec3(0.0));
  std::vector<Ume::Vec3> want_vdiff(nbin, Ume::Vec3(0.0));
  for (int i = 0; i < nadd; ++i) {
    want_count[i % nbin] += 1;
    want_sum[i % nbin] += i % 9;
    want_vsum[i % nbin] += term(i);
    want_vdiff[i % nbin] -= term(i);
  }

  /* Positive control: one add lost from one bin is one wrong bin. */
  std::vector<Ume::Vec3> one_lost = want_vsum;
  one_lost[nbin - 1] -= term(nadd - 1);
  REQUIRE(count_wrong(one_lost, want_vsum) == 1);

  for (int rep = 0; rep < repetitions; ++rep) {
    std::vector<int> count(nbin, 0);
    std::vector<double> sum(nbin, 0.0);
    std::vector<Ume::Vec3> vsum(nbin, Ume::Vec3(0.0));
    std::vector<Ume::Vec3> vdiff(nbin, Ume::Vec3(0.0));
    Kokkos::View<int *, HostSpace> h_count(count.data(), count.size());
    Kokkos::View<double *, HostSpace> h_sum(sum.data(), sum.size());
    Kokkos::View<Ume::Vec3 *, HostSpace> h_vsum(vsum.data(), vsum.size());
    Kokkos::View<Ume::Vec3 *, HostSpace> h_vdiff(vdiff.data(), vdiff.size());

    Kokkos::parallel_for("test_atomic_add",
        Kokkos::RangePolicy<HostExecSpace>(0, nadd), [&](int const i) {
          int const b = i % nbin;
          Ume::ume_atomic_add(&h_count(b), 1);
          Ume::ume_atomic_add(&h_sum(b), static_cast<double>(i % 9));
          Ume::ume_atomic_add(&h_vsum(b), term(i));
          Ume::ume_atomic_add(&h_vdiff(b), term(i) * -1.0);
        });
    Kokkos::fence();

    INFO("repetition " << rep);
    CHECK(count_wrong(count, want_count) == 0);
    CHECK(count_wrong(sum, want_sum) == 0);
    CHECK(count_wrong(vsum, want_vsum) == 0);
    CHECK(count_wrong(vdiff, want_vdiff) == 0);
  }
}
