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
  \file Ume/RaggedRight.hh
*/

#ifndef UME_RAGGEDRIGHT_HH
#define UME_RAGGEDRIGHT_HH

#include "shm_allocator.hh"
#include <span>
#include <vector>

namespace Ume {

//! An array-of-arrays, where each array can be a different length
/*! Implemented as three arrays: the data for primary index `n` is stored in the
    array `data`, in the half-open interval [bidx[n]..eidx[n]).
 */
template <class T> struct RaggedRight {
  RaggedRight() = default;
  explicit RaggedRight(int base_size) { init(base_size); }
  void init(int const len) {
    bidx.assign(len, 0);
    eidx.assign(len, 0);
  }

  bool operator==(RaggedRight<T> const &rhs) const {
    return (bidx == rhs.bidx && eidx == rhs.eidx && data == rhs.data);
  }

  std::span<T> operator[](int const n) {
    return std::span(data.begin() + bidx[n], size(n));
  }
  std::span<T const> const operator[](int const n) const {
    return std::span(data.cbegin() + bidx[n], size(n));
  }

  //! Copy the contents of the range [`b`..`e`) to element `n`
  /*! Note that if `n` has already been assigned data, it will be abandonend in
      the `data` array! */
  template <class IT> void assign(int const n, IT const b, IT const e) {
    bidx.at(n) = static_cast<int>(data.size());
    data.insert(data.end(), b, e);
    eidx.at(n) = static_cast<int>(data.size());
  }

  //! Return the length of the n'th array
  constexpr int size(int const n) const { return eidx[n] - bidx[n]; }

private:
  UmeVector<int> bidx, eidx;
  UmeVector<T> data;
};

} // namespace Ume

#endif
