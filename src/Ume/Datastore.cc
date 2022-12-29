/*
\file Datastore.cc
*/

#include "Datastore.hh"
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace Ume {

void DS_Entry::set_type(DS_Entry::Types type) {
  type_ = type;
  switch (type) {
  case Types::INT:
    data_ = INT_T();
    break;
  case Types::INTV:
    data_ = INTV_T();
    break;
  case Types::DBL:
    data_ = DBL_T();
    break;
  case Types::DBLV:
    data_ = DBLV_T();
    break;
  case Types::VEC3:
    data_ = VEC3_T();
    break;
  case Types::VEC3V:
    data_ = VEC3V_T();
    break;
  case Types::NONE:
    break;
  }
}

Datastore::dsptr Datastore::add_child_() {
  children_.emplace_back(new Datastore());
  children_.back()->parent_ = getptr();
  return children_.back();
}

DS_Entry *Datastore::find(std::string const &name) {
  auto it = entries_.find(name);
  if (it != entries_.end()) {
    return it->second.get();
  }
  if (!parent_.expired())
    return parent_.lock()->find(name);
  return nullptr;
}

DS_Entry const *Datastore::cfind(std::string const &name) const {
  auto const it = entries_.find(name);
  if (it != entries_.cend()) {
    return it->second.get();
  }
  if (!parent_.expired())
    return parent_.lock()->cfind(name);
  return nullptr;
}

DS_Entry *Datastore::find_or_die(std::string const &name) {
  DS_Entry *ptr = find(name);
  if (!ptr) {
    std::cerr << "Error: unable to find datastore variable named "
              << std::quoted(name) << std::endl;
    std::abort();
  }
  return ptr;
}

DS_Entry const *Datastore::cfind_or_die(std::string const &name) const {
  DS_Entry const *ptr = cfind(name);
  if (!ptr) {
    std::cerr << "Error: unable to find datastore variable named "
              << std::quoted(name) << std::endl;
    std::abort();
  }
  return ptr;
}

Datastore::~Datastore() {
  for (auto &p : children_) {
    p.reset();
  }
}

} // namespace Ume
