/*
  \file Datastore.hh
*/

#ifndef DATASTORE_HH
#define DATASTORE_HH

#include <memory>
#include <unordered_map>
#include <vector>

namespace Ume {

class Datastore : public std::enable_shared_from_this<Datastore> {
public:
  using sptr = std::shared_ptr<Datastore>;

public:
  sptr getptr() { return shared_from_this(); }
  [[nodiscard]] static sptr create_root() { return sptr(new Datastore()); }
  [[nodiscard]] static sptr create_child(sptr parent) {
    return parent->add_child_();
  }

  ~Datastore();

private:
  // Force the use of the factory function by making the ctors private
  Datastore() = default;
  sptr add_child_();

public:
  std::weak_ptr<Datastore> parent_;
  std::vector<sptr> children_;
};

} // namespace Ume

#endif
