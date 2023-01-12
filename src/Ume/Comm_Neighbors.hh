/*!
\file Ume/Comm_Neighbors.hh
*/

#ifndef COMM_NEIGHBORS_HH
#define COMM_NEIGHBORS_HH 1

#include <vector>

namespace Ume {
namespace Comm {

//! Identify the remote PE and communicating elements in a one-way pattern
/*! For sends, \p elements is a list of entity (zones, points, etc) indices that
    we are gathering into the send buffer.  For receives, it is a scatter
    pattern from the buffer into the entity. */
struct Neighbor {
  int pe;
  std::vector<int> elements;
  bool operator==(Neighbor const &rhs) const {
    return (pe == rhs.pe && elements == rhs.elements);
  }
};

using Neighbors = std::vector<Neighbor>;

} // namespace Comm
} // namespace Ume

#endif
