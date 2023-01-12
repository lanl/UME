/*
  \file Datastore.hh
*/

#ifndef DATASTORE_HH
#define DATASTORE_HH

#include "Ume/VecN.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Ume {

class Datastore;

struct DS_Types {
  enum class Types { INT, INTV, DBL, DBLV, VEC3, VEC3V, NONE };
  using INT_T = int;
  using INTV_T = std::vector<INT_T>;
  using DBL_T = double;
  using DBLV_T = std::vector<DBL_T>;
  using VEC3_T = Vec3;
  using VEC3V_T = std::vector<VEC3_T>;
};

class DS_Entry : public DS_Types {
public:
  DS_Entry() = default;
  explicit DS_Entry(Types t) { set_type(t); }
  virtual ~DS_Entry() = default;
  void set_type(Types t);

protected:
  friend class Datastore;
  Types type_;
  //  std::weak_ptr<Datastore> ds_;
  mutable std::variant<INT_T, INTV_T, DBL_T, DBLV_T, VEC3_T, VEC3V_T> data_;
  mutable bool dirty_ = false;
  enum class Init_State { UNINITIALIZED, IN_PROGRESS, INITIALIZED };
  mutable Init_State init_state_{Init_State::UNINITIALIZED};

protected:
  virtual bool init_() const {
    init_state_ = Init_State::INITIALIZED;
    return false;
  }
};

class Datastore : public DS_Types {
public:
  using dsptr = std::unique_ptr<Datastore>;
  using eptr = std::unique_ptr<DS_Entry>;

public:
  [[nodiscard]] static dsptr create_root() {
    return dsptr(new Datastore("root"));
  }
  [[nodiscard]] static Datastore *create_child(
      Datastore *parent, char const *const name) {
    return parent->add_child_(name);
  }

  constexpr std::string const &name() const { return name_; }
  std::string path() const {
    if (parent_)
      return parent_->path() + "/" + name();
    return "/" + name();
  }

  bool insert(char const *const name, eptr &&ptr) {
    auto res =
        entries_.emplace(std::make_pair(std::string{name}, std::move(ptr)));
    return res.second;
  }
#define MAKE_ACCESS(Y, T) \
  inline T &access_##Y(char const *const name) { \
    auto ptr = find_or_die(name); \
    ptr->init_(); \
    ptr->dirty_ = true; \
    return std::get<T>(ptr->data_); \
  } \
  inline T const &caccess_##Y(char const *const name) const { \
    auto ptr = cfind_or_die(name); \
    ptr->dirty_ = ptr->init_(); \
    return std::get<T>(ptr->data_); \
  }

  MAKE_ACCESS(int, INT_T);
  MAKE_ACCESS(intv, INTV_T);
  MAKE_ACCESS(dbl, DBL_T);
  MAKE_ACCESS(dblv, DBLV_T);
  MAKE_ACCESS(vec3, VEC3_T);
  MAKE_ACCESS(vec3v, VEC3V_T);

#undef MAKE_ACCESS

  ~Datastore();

private:
  // Force the use of the factory function by making the ctors private
  Datastore() = delete;
  explicit Datastore(char const *const name) : name_{name}, parent_{nullptr} {}
  Datastore *add_child_(char const *const name);
  [[nodiscard]] DS_Entry *find(std::string const &name);
  [[nodiscard]] DS_Entry const *cfind(std::string const &name) const;
  [[nodiscard]] DS_Entry *find_or_die(std::string const &name);
  [[nodiscard]] DS_Entry const *cfind_or_die(std::string const &name) const;

private:
  std::unordered_map<std::string, eptr> entries_;
  std::string name_;

public: /* These are public for testing purposes */
  Datastore *parent_;
  std::vector<dsptr> children_;
};

} // namespace Ume

#endif
