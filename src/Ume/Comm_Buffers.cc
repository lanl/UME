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
  \file Ume/Comm_Buffers.cc

  Implementation and explicit instantiation for Buffers<T>.
*/

#include "Ume/Comm_Buffers.hh"
#include <cassert>
#include <set>

namespace Ume {
namespace Comm {

template <class T>
Buffers<T>::Buffers(Neighbors const &neighs) : remotes{neighs.size()} {
  auto const entries_per_elem = DS_Type_Info<T>::elem_len;
  size_t count{0};
  for (size_t ni = 0; ni < neighs.size(); ++ni) {
    // Set the remote PE
    remotes[ni].pe = neighs[ni].pe;
    // Set the offset of this PE's data into the aggregated buffer
    remotes[ni].buf_offset = count * entries_per_elem;
    // Set the length of this PE's data in the aggregated buffer
    remotes[ni].buf_len =
        static_cast<int>(neighs[ni].elements.size() * entries_per_elem);
    // Add these elements to the bufIdx_to_EntIdx map
    b2e_m.insert(
        b2e_m.end(), neighs[ni].elements.begin(), neighs[ni].elements.end());
    // Advance the total count
    count += neighs[ni].elements.size();
  }
  // Resize the aggregated buffer
  buf.resize(count * entries_per_elem);
}

template <class T> void Buffers<T>::pack(T const &field) {
  const size_t N = num_entries();
  base_type *buf = get_buf();
  size_t const *map = buf2ent();
  for (size_t i = 0; i < N; ++i) {
    buf = Comm::pack(field[map[i]], buf);
  }
}

template <class T> void Buffers<T>::unpack(T &field, Op op) {
  const size_t N = num_entries();
  base_type const *buf = get_buf();
  size_t const *map = buf2ent();

  switch (op) {
  case Op::MAX: {
    typename T::value_type val;
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::unpack(buf, val);
      field[map[i]] = std::max(field[map[i]], val);
    }
  } break;
  case Op::MIN: {
    typename T::value_type val;
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::unpack(buf, val);
      field[map[i]] = std::min(field[map[i]], val);
    }
  } break;
  case Op::OVERWRITE: {
    /* This includes a sanity check to make sure an entry is not overwritten
       more than once.  This should be removed when everything is working */
    std::set<size_t> entries; // debug
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::unpack(buf, field[map[i]]);
      assert(entries.count(map[i]) == 0); // debug
      entries.insert(map[i]); // debug
    }
  } break;
  case Op::SUM: {
    typename T::value_type val;
    for (size_t i = 0; i < N; ++i) {
      buf = Comm::unpack(buf, val);
      field[map[i]] += val;
    }
  } break;
  }
}

template class Buffers<DS_Types::INTV_T>;
template class Buffers<DS_Types::DBLV_T>;
template class Buffers<DS_Types::VEC3V_T>;

} // namespace Comm
} // namespace Ume
