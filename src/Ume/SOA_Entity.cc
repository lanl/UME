#include "SOA_Entity.hh"
#include "utils.hh"

namespace Ume {

//! Binary write for std::vector<Entity::Subset>
template <>
void write_bin<Ume::SOA_Idx::Entity::Subset>(
    std::ostream &os, std::vector<Ume::SOA_Idx::Entity::Subset> const &data) {
  write_bin(os, data.size());
  if (!data.empty()) {
    for (auto const &c : data) {
      write_bin(os, c.name);
      write_bin(os, c.lsize);
      write_bin(os, c.elements);
      write_bin(os, c.mask);
      os << '\n';
    }
  }
  os << '\n';
}

//! Binary read for std::vector<Entity::Subset>
template <>
void read_bin<Ume::SOA_Idx::Entity::Subset>(
    std::istream &is, std::vector<Ume::SOA_Idx::Entity::Subset> &data) {
  size_t len;
  read_bin(is, len);
  if (len == 0) {
    data.clear();
  } else {
    data.resize(len);
    for (size_t i = 0; i < len; ++i) {
      read_bin(is, data[i].name);
      read_bin(is, data[i].lsize);
      read_bin(is, data[i].elements);
      read_bin(is, data[i].mask);
      Ume::skip_line(is);
    }
  }
  Ume::skip_line(is);
}

} // namespace Ume

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Entity -------------------------------- */

void Entity::write(std::ostream &os) const {
  write_bin(os, lsize);
  write_bin(os, mask);
  write_bin(os, comm_type);
  write_bin(os, cpy_idx);
  write_bin(os, src_pe);
  write_bin(os, src_idx);
  write_bin(os, ghost_mask);
  write_bin<Comm::Neighbors>(os, recvFrom);
  write_bin<Comm::Neighbors>(os, sendTo);
  write_bin(os, subsets);
  os << '\n';
}

void Entity::read(std::istream &is) {
  read_bin(is, lsize);
  read_bin(is, mask);
  read_bin(is, comm_type);
  read_bin(is, cpy_idx);
  read_bin(is, src_pe);
  read_bin(is, src_idx);
  read_bin(is, ghost_mask);
  read_bin<Comm::Neighbors>(is, recvFrom);
  read_bin<Comm::Neighbors>(is, sendTo);
  read_bin(is, subsets);
  skip_line(is);
}

bool Entity::operator==(Entity const &rhs) const {
  return (lsize == rhs.lsize && mask == rhs.mask &&
      comm_type == rhs.comm_type && cpy_idx == rhs.cpy_idx &&
      src_pe == rhs.src_pe && src_idx == rhs.src_idx &&
      ghost_mask == rhs.ghost_mask && recvFrom == rhs.recvFrom &&
      sendTo == rhs.sendTo && subsets == rhs.subsets);
}

void Entity::resize(int const local, int const total, int const ghost) {
  mask.resize(total);
  comm_type.resize(total);
  cpy_idx.resize(ghost);
  src_pe.resize(ghost);
  src_idx.resize(ghost);
  ghost_mask.resize(ghost);
  lsize = local;
}

} // namespace SOA_Idx
} // namespace Ume
