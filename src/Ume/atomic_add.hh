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
  \file Ume/atomic_add.hh

  The add a Kokkos kernel uses to accumulate into an element that other
  iterations of the same parallel loop may also be adding into.
*/

#ifndef UME_ATOMIC_ADD_HH
#define UME_ATOMIC_ADD_HH 1

#include "Ume/VecN.hh"
#include <Kokkos_Core.hpp>
#include <cstddef>
#include <type_traits>

namespace Ume {

/*! Add `val` to `*dest`.  With UME_SERIAL every execution space is
    Kokkos::Serial (mem_exec_spaces.hh), so a plain add is enough; otherwise
    the loop may run on a threaded or device backend and the add is atomic.
    The type is deduced from `dest` alone, so `val` converts to it. */
template <class T>
  requires std::is_arithmetic_v<T>
KOKKOS_INLINE_FUNCTION void ume_atomic_add(
    T *const dest, std::type_identity_t<T> const val) {
#if defined(UME_SERIAL)
  *dest += val;
#else
  Kokkos::atomic_add(dest, val);
#endif
}

/*! Add `val` to `*dest` one component at a time.  desul has no lock-free
    atomic for a 24-byte Vec3 and would serialize each whole-vector add on a
    lock from its global table; each scalar add here is lock-free.  A reader
    running concurrently could see some components added and not others, which
    nothing reads while a loop is accumulating. */
template <class T, std::size_t N>
KOKKOS_INLINE_FUNCTION void ume_atomic_add(
    VecN<T, N> *const dest, std::type_identity_t<VecN<T, N>> const &val) {
  for (std::size_t i = 0; i < N; ++i)
    ume_atomic_add(&(*dest)[i], val[i]);
}

} // namespace Ume

#endif
