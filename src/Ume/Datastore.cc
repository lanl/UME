/*
\file Datastore.cc
*/

#include "Datastore.hh"

namespace Ume {

Datastore::sptr Datastore::add_child_() {
  children_.emplace_back(new Datastore());
  children_.back()->parent_ = getptr();
  return children_.back();
}

Datastore::~Datastore() {
  for (auto &p : children_) {
    p.reset();
  }
}

} // namespace Ume
