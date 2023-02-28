/*!
\file Ume/Comm_Neighbors.hh
*/

#ifndef UME_COMM_NEIGHBORS_HH
#define UME_COMM_NEIGHBORS_HH 1

#include "Ume/utils.hh"
#include <vector>

namespace Ume {

//! Namespace for routines dealing with communication between mesh partitions
namespace Comm {

//! Identify the remote PE and communicating elements in a one-way pattern
/*! For sends, \p elements is a list of entity (zones, points, etc) indices that
    we are gathering into the send buffer.  For receives, it is a scatter
    pattern from the buffer into the entity. */
struct Neighbor {
  int pe;
  std::vector<int> elements;
  constexpr bool operator==(Neighbor const &rhs) const {
    return (pe == rhs.pe && elements == rhs.elements);
  }
};

using Neighbors = std::vector<Neighbor>;

} // namespace Comm

//! Binary write specialization for Ume::Comm::Neighbors
template <>
void write_bin<Comm::Neighbors>(
    std::ostream &os, Ume::Comm::Neighbors const &nvec);

//! Binary read specialization for Ume::Comm::Neighbors
template <>
void read_bin<Comm::Neighbors>(std::istream &is, Ume::Comm::Neighbors &nvec);

} // namespace Ume

#endif
