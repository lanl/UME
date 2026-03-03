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
\file Ume/array_types.hh
*/

#ifndef UME_ARRAY_TYPES_HH
#define UME_ARRAY_TYPES_HH

#include "memory.hh"

/* Basic scratch array types -- memory managed by Kokkos */

template <typename T>
using SArrayRank1 = Kokkos::View<T *, DefaultMemSpace>;

template <typename T>
using SArrayRank2 =
    Kokkos::View<T **, MemLayout, DefaultMemSpace>;

template <typename T>
using SArrayRank3 =
    Kokkos::View<T ***, MemLayout, DefaultMemSpace>;

template <typename T>
using SArrayRank4 =
    Kokkos::View<T ****, MemLayout, DefaultMemSpace>;

#endif
