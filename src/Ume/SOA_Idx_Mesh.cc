/*!
  \file SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include <istream>
#include <ostream>

namespace Ume {
namespace SOA_Idx {
void Corners::write(std::ostream &os) const {}
void Corners::read(std::istream &is) {}
void Edges::write(std::ostream &os) const {}
void Edges::read(std::istream &is) {}
void Faces::write(std::ostream &os) const {}
void Faces::read(std::istream &is) {}
void Points::write(std::ostream &os) const {}
void Points::read(std::istream &is) {}
void Sides::write(std::ostream &os) const {}
void Sides::read(std::istream &is) {}
void Zones::write(std::ostream &os) const {}
void Zones::read(std::istream &is) {}
} // namespace SOA_Idx
} // namespace Ume
