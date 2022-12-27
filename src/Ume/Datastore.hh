/*
  \file Datastore.hh
*/

#ifndef DATASTORE_HH
#define DATASTORE_HH

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ume {

class Datastore : public std::enable_shared_from_this<Datastore> {
public:
  using sptr = std::shared_ptr<Datastore>;
  enum class Types { INT, INTV, DBL, DBLV, NONE };

public:
  sptr getptr() { return shared_from_this(); }
  [[nodiscard]] static sptr create_root() { return sptr(new Datastore()); }
  [[nodiscard]] static sptr create_child(sptr parent) {
    return parent->add_child_();
  }

  std::pair<Types, sptr> find(std::string const &name);
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
  sptr add_child_();
  std::unordered_map<std::string, int> int_vars_;
  std::unordered_map<std::string, std::vector<int>> vint_vars_;
  std::unordered_map<std::string, double> dbl_vars_;
  std::unordered_map<std::string, std::vector<double>> vdbl_vars_;

public:
  std::weak_ptr<Datastore> parent_;
  std::vector<sptr> children_;
};

} // namespace Ume

#endif
