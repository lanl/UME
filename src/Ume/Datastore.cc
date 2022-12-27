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

std::pair<Datastore::Types, Datastore::sptr> Datastore::find(
    std::string const &name) {
  if (int_vars_.count(name))
    return std::make_pair(Types::INT, getptr());
  if (vint_vars_.count(name))
    return std::make_pair(Types::INTV, getptr());
  if (dbl_vars_.count(name))
    return std::make_pair(Types::DBL, getptr());
  if (vdbl_vars_.count(name))
    return std::make_pair(Types::DBLV, getptr());
  if (!parent_.expired())
    return parent_.lock()->find(name);
  return std::make_pair(Types::NONE, sptr());
}

void Datastore::set(std::string const &name, int const val) {
  auto it = int_vars_.find(name);
  if (it == int_vars_.end()) {
    int_vars_.insert(std::make_pair(name, val));
  } else {
    it->second = val;
  }
}

void Datastore::set(std::string const &name, std::vector<int> &val) {
  auto res = vint_vars_.insert(std::make_pair(name, std::vector<int>()));
  auto it = res.first;
  it->second.swap(val);
}

void Datastore::set(std::string const &name, double const val) {
  auto it = dbl_vars_.find(name);
  if (it == dbl_vars_.end()) {
    dbl_vars_.insert(std::make_pair(name, val));
  } else {
    it->second = val;
  }
}

void Datastore::set(std::string const &name, std::vector<double> &val) {
  auto res = vdbl_vars_.insert(std::make_pair(name, std::vector<double>()));
  auto it = res.first;
  it->second.swap(val);
}

Datastore::~Datastore() {
  for (auto &p : children_) {
    p.reset();
  }
}

} // namespace Ume
