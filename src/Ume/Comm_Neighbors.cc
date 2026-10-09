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
\file Ume/Comm_Neighbors.cc
*/

#include "Ume/Comm_Neighbors.hh"
#include <algorithm>

namespace Ume {

bool Comm::elements_unique(Neighbors const &neighs) {
  std::vector<int> all;
  for (auto const &n : neighs)
    all.insert(all.end(), n.elements.begin(), n.elements.end());
  std::ranges::sort(all);
  return std::ranges::adjacent_find(all) == all.end();
}

template <>
void write_bin<Comm::Neighbors>(
    std::ostream &os, Ume::Comm::Neighbors const &nvec) {
  write_bin(os, std::string{"neighbors"});
  write_bin(os, nvec.size());
  if (!nvec.empty()) {
    for (auto const &n : nvec) {
      write_bin(os, n.pe);
      write_bin(os, n.elements);
      os << '\n';
    }
  }
  os << '\n';
}

template <>
void read_bin<Comm::Neighbors>(std::istream &is, Ume::Comm::Neighbors &nvec) {
  if (!read_tag(is, "neighbors"))
    return;
  size_t len{0};
  read_bin(is, len);
  // A stream that ends inside the length word leaves the bytes it got in
  // `len`; a failed read is an empty one.
  if (!is || len == 0) {
    Ume::Comm::Neighbors foo;
    nvec.swap(foo);
  } else {
    nvec.resize(len);
    for (size_t i = 0; i < len; ++i) {
      read_bin(is, nvec[i].pe);
      read_bin(is, nvec[i].elements);
      Ume::skip_line(is);
    }
  }
  Ume::skip_line(is);
}

} // namespace Ume
