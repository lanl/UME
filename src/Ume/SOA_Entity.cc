/*!
  \file Ume/SOA_Entity.cc
*/
#include "Ume/SOA_Entity.hh"
#include "Ume/utils.hh"
#include <cassert>

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
  write_bin<Comm::Neighbors>(os, myCpys);
  write_bin<Comm::Neighbors>(os, mySrcs);
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
  read_bin<Comm::Neighbors>(is, myCpys);
  read_bin<Comm::Neighbors>(is, mySrcs);
  read_bin(is, subsets);
  skip_line(is);
}

bool Entity::operator==(Entity const &rhs) const {
  return (lsize == rhs.lsize && mask == rhs.mask &&
      comm_type == rhs.comm_type && cpy_idx == rhs.cpy_idx &&
      src_pe == rhs.src_pe && src_idx == rhs.src_idx &&
      ghost_mask == rhs.ghost_mask && myCpys == rhs.myCpys &&
      mySrcs == rhs.mySrcs && subsets == rhs.subsets);
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

template <typename FT> void Entity::gather(Comm::Op const op, FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> cpyBufs(myCpys);
  Comm::Buffers<FT> srcBufs(mySrcs);
  cpyBufs.pack(field);
  comm().exchange(cpyBufs, srcBufs); // send local copies to remote sources
  srcBufs.unpack(field, op);
}

template <typename FT> void Entity::scatter(FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> cpyBufs(myCpys);
  Comm::Buffers<FT> srcBufs(mySrcs);
  srcBufs.pack(field);
  comm().exchange(srcBufs, cpyBufs); // send local sources to remote copies
  cpyBufs.unpack(field, Comm::Op::OVERWRITE);
}

template <typename FT> void Entity::gathscat(Comm::Op const op, FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> cpyBufs(myCpys);
  Comm::Buffers<FT> srcBufs(mySrcs);
  cpyBufs.pack(field);
  comm().exchange(cpyBufs, srcBufs);
  /* srcBufs now contains the remote copy values */
  srcBufs.unpack(field, op);
  srcBufs.pack(field); // refill from merged field
  /* srcBufs now contains the the merged source values */
  comm().exchange(srcBufs, cpyBufs);
  cpyBufs.unpack(field, Comm::Op::OVERWRITE);
}

template void Entity::gather<DS_Types::INTV_T>(
    Comm::Op const op, DS_Types::INTV_T &field);
template void Entity::scatter<DS_Types::INTV_T>(DS_Types::INTV_T &field);
template void Entity::gathscat<DS_Types::INTV_T>(
    Comm::Op const op, DS_Types::INTV_T &field);

template void Entity::gather<DS_Types::DBLV_T>(
    Comm::Op const op, DS_Types::DBLV_T &field);
template void Entity::scatter<DS_Types::DBLV_T>(DS_Types::DBLV_T &field);
template void Entity::gathscat<DS_Types::DBLV_T>(
    Comm::Op const op, DS_Types::DBLV_T &field);

template void Entity::gather<DS_Types::VEC3V_T>(
    Comm::Op const op, DS_Types::VEC3V_T &field);
template void Entity::scatter<DS_Types::VEC3V_T>(DS_Types::VEC3V_T &field);
template void Entity::gathscat<DS_Types::VEC3V_T>(
    Comm::Op const op, DS_Types::VEC3V_T &field);

} // namespace SOA_Idx
} // namespace Ume
