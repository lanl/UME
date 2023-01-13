/*!
\file Ume/Comm_Neighbors.cc
*/

#include "Ume/Comm_Neighbors.hh"

namespace Ume {

template <>
void write_bin<Comm::Neighbors>(
    std::ostream &os, Ume::Comm::Neighbors const &nvec) {
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
  size_t len;
  read_bin(is, len);
  if (len == 0) {
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
