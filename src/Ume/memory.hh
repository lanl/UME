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

/* NOTE: We use left layout to avoid transposing arrays across
 * Fortran/C++ language barriers. For maximum performance, we
 * should be using right layout but the size of the "bad" dimension
 * is usuallly small. */
using MemLayout = Kokkos::LayoutLeft;

namespace MemOpts {
struct DoNotCopyInit : std::false_type {};
struct CopyInit : std::true_type {};
}

#endif
