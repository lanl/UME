/*
  \file Datastore.hh
*/

#ifndef DATASTORE_HH
#define DATASTORE_HH

#include "Ume/VecN.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ume {

class Datastore : public std::enable_shared_from_this<Datastore> {
public:
  using dsptr = std::shared_ptr<Datastore>;
  enum class Types { INT, INTV, DBL, DBLV, VEC3, VEC3V, NONE };

public:
  dsptr getptr() { return shared_from_this(); }
  [[nodiscard]] static dsptr create_root() { return dsptr(new Datastore()); }
  [[nodiscard]] static dsptr create_child(dsptr parent) {
    return parent->add_child_();
  }

  std::pair<Types, dsptr> find(std::string const &name);
  void set(std::string const &name, int const val);
  // This destoys val!
  void set(std::string const &name, std::vector<int> &val);
  void set(std::string const &name, double const val);
  // This destoys val!
  void set(std::string const &name, std::vector<double> &val);

  ~Datastore();

private:
  // Force the use of the factory function by making the ctors private
  Datastore() = default;

  dsptr add_child_();
  std::unordered_map<std::string, int> int_vars_;
  std::unordered_map<std::string, std::vector<int>> vint_vars_;
  std::unordered_map<std::string, double> dbl_vars_;
  std::unordered_map<std::string, std::vector<double>> vdbl_vars_;
  std::unordered_map<std::string, VecN<double, 3>> vec3_vars_;
  std::unordered_map<std::string, std::vector<VecN<double, 3>>> vvec3_vars_;

public:
  std::weak_ptr<Datastore> parent_;
  std::vector<dsptr> children_;
};

} // namespace Ume

#endif
