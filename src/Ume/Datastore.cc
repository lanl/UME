/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*
\file Ume/Datastore.cc
*/

#include "Ume/Datastore.hh"
#include <cstdlib>
#include <iomanip>

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
  case Types::INTRR:
    data_ = INTRR_T();
    break;
  case Types::DBL:
    data_ = DBL_T();
    break;
  case Types::DBLV:
    data_ = DBLV_T();
    break;
  case Types::DBLRR:
    data_ = DBLRR_T();
    break;
  case Types::VEC3:
    data_ = VEC3_T();
    break;
  case Types::VEC3V:
    data_ = VEC3V_T();
    break;
  case Types::VEC3RR:
    data_ = VEC3RR_T();
  case Types::NONE:
    break;
  }
}

Datastore *Datastore::add_child_(char const *const name) {
  children_.emplace_back(new Datastore(name));
  children_.back()->parent_ = this;
  return children_.back().get();
}

DS_Entry *Datastore::find(std::string const &name) {
  auto it = entries_.find(name);
  if (it != entries_.end()) {
    return it->second.get();
  }
  if (parent_)
    return parent_->find(name);
  return nullptr;
}

DS_Entry const *Datastore::cfind(std::string const &name) const {
  auto const it = entries_.find(name);
  if (it != entries_.cend()) {
    return it->second.get();
  }
  if (parent_)
    return parent_->cfind(name);
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
