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

/* Memory pool scratch array allocators. These optionally fall back to the basic
 * Kokkos view scratch array types. */

template <typename T, bool useMemPool = true,
          typename CopyOpt = MemOpts::CopyInit>
inline auto NewArrayRank1(const std::string &name, const int dim0,
                          const T init_value = static_cast<T>(0),
                          CopyOpt &&opt = MemOpts::CopyInit{}) {
  if constexpr (useMemPool)
    return GetMemPool().GetView<T>(
        init_value, dim0, std::forward<CopyOpt>(opt));
  else
    return SArrayRank1<T>(name, static_cast<size_t>(dim0));
};

template <typename T, bool useMemPool = true,
          typename CopyOpt = MemOpts::CopyInit>
inline auto NewArrayRank2(const std::string &name, const int dim0,
                          const int dim1,
                          const T init_value = static_cast<T>(0),
                          CopyOpt &&opt = MemOpts::CopyInit{}) {
  if constexpr (useMemPool)
    return GetMemPool().GetView<T>(
        init_value, dim0, dim1, std::forward<CopyOpt>(opt));
  else
    return SArrayRank2<T>(
        name, static_cast<size_t>(dim0), static_cast<size_t>(dim1));
};

template <typename T, bool useMemPool = true,
          typename CopyOpt = MemOpts::CopyInit>
inline auto NewArrayRank3(const std::string &name, const int dim0,
                          const int dim1, const int dim2,
                          const T init_value = static_cast<T>(0),
                          CopyOpt &&opt = MemOpts::CopyInit{}) {
  if constexpr (useMemPool)
    return GetMemPool().GetView<T>(
        init_value, dim0, dim1, dim2, std::forward<CopyOpt>(opt));
  else
    return SArrayRank3<T>(name, static_cast<size_t>(dim0),
        static_cast<size_t>(dim1), static_cast<size_t>(dim2));
};

template <typename T, bool useMemPool = true,
          typename CopyOpt = MemOpts::CopyInit>
inline auto NewArrayRank4(const std::string &name, const int dim0,
                          const int dim1, const int dim2, const int dim3,
                          const T init_value = static_cast<T>(0),
                          CopyOpt &&opt = MemOpts::CopyInit{}) {
  if constexpr (useMemPool)
    return GetMemPool().GetView<T>(
        init_value, dim0, dim1, dim2, dim3, std::forward<CopyOpt>(opt));
  else
    return SArrayRank4<T>(name, static_cast<size_t>(dim0),
        static_cast<size_t>(dim1), static_cast<size_t>(dim2),
        static_cast<size_t>(dim3));
};

/* Array typedefs for mempool return types */

template <typename T>
using ArrayRank1 = Kokkos::View<T *, DefaultMemSpace, MemUnmanaged>;

template <typename T>
using ArrayRank2 = Kokkos::View<T **, MemLayout, DefaultMemSpace, MemUnmanaged>;

template <typename T>
using ArrayRank3 =
    Kokkos::View<T ***, MemLayout, DefaultMemSpace, MemUnmanaged>;

template <typename T>
using ArrayRank4 =
    Kokkos::View<T ****, MemLayout, DefaultMemSpace, MemUnmanaged>;

#endif
