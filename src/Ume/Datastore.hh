/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
  \file Ume/Datastore.hh
*/

#ifndef UME_DATASTORE_HH
#define UME_DATASTORE_HH 1

#include "Ume/DS_Types.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <iostream>

namespace Ume {

class Datastore;

//! Datastore entry
/*! Basically a variant for all possible types that can be stored in the
    Datastore, plus some metadata. DS_Entry generally serves as a base class for
    actual variable classes. See Entity_Field for more details. */
class DS_Entry : public DS_Types {
public:
  DS_Entry() = default;
  //! Set the type of the data
  explicit DS_Entry(Types t) { set_type(t); }
  virtual ~DS_Entry() = default;

  //! Set the type of the data held in this entry
  void set_type(Types t);

protected:
  friend class Datastore;

  //! The DS_Type that is currently stored in the `data_` variant
  Types type_;

  //! The DS_Types that can be stored in an DS_Entry
  mutable std::variant<INT_T, INTV_T, INTRR_T, DBL_T, DBLV_T, DBLRR_T, VEC3_T,
      VEC3V_T, VEC3RR_T>
      data_;

  //! This is changed to true when accessed through a non-const access stmt
  mutable bool dirty_ = false;

  //! A list of states that this entry can be in
  /*! The IN_PROGRESS state is set at the begining of the initialization
      process.  It serves to protect against initialization loops, where two
      variables attempt to access each other during their own initialization
      process. */
  enum class Init_State { UNINITIALIZED, IN_PROGRESS, INITIALIZED };

  //! The current initialization state
  mutable Init_State init_state_{Init_State::UNINITIALIZED};

protected:
  //! Default initialization call
  /*! Each derived version of this class should provide an `init_()` function
    that is called when the DS_Entry is accessed in an UNINITIALIZED state. */
  virtual bool init_() const {
    init_state_ = Init_State::INITIALIZED;
    return false;
  }
};

//! A hierarchical key-value data storage class
/*! This class implements a tree of key-value datastores, binding string names
    to DS_Entry types.
 */
class Datastore : public DS_Types {
public:
  using dsptr = std::unique_ptr<Datastore>;
  using eptr = std::unique_ptr<DS_Entry>;

public:
  //! Factory method for creating a root datastore.
  [[nodiscard]] static dsptr create_root() {
    return dsptr(new Datastore("root"));
  }

  //! Factory method for creating a new datastore under a parent
  /*! The new datastore will have the name `name` and will be placed in
    the `children_` list of `parent`. */
  [[nodiscard]] static Datastore *create_child(
      Datastore *parent, char const *const name) {
    return parent->add_child_(name);
  }

  //! Return the name of this datastore
  constexpr std::string const &name() const { return name_; }

  //! Return the chain of datastore names from the root to this child.
  std::string path() const {
    if (parent_)
      return parent_->path() + "/" + name();
    return "/" + name();
  }

  //! Add a new named DS_Entry, return true on success
  bool insert(char const *const name, eptr &&ptr) {
    auto res =
        entries_.emplace(std::make_pair(std::string{name}, std::move(ptr)));
    return res.second;
  }

  //! A macro that creates a type-specific access function (and its const ver)
  /*! MAKE_ACCESS(int, INT_T) will create two functions:
           access_int(name)
           caccess_int(name)
      These are used to lookup keys in the datastore and (in this case) return
      an scalar integer value.  Lookups occur in a hierarchical fashion,
      starting at the current datastore and heading up to the root.
  */
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
  MAKE_ACCESS(intrr, INTRR_T);
  MAKE_ACCESS(dbl, DBL_T);
  MAKE_ACCESS(dblv, DBLV_T);
  MAKE_ACCESS(dblrr, DBLRR_T);
  MAKE_ACCESS(vec3, VEC3_T);
  MAKE_ACCESS(vec3v, VEC3V_T);
  MAKE_ACCESS(vec3rr, VEC3RR_T);

#undef MAKE_ACCESS

  //! Recursively delete this tree and its children.
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
  //! The actual datastore
  std::unordered_map<std::string, eptr> entries_;
  //! The name of this datastore
  std::string name_;

public:
  //! Return a list of "to" maps for a mesh entity.
  void to_entity_map_names(std::string const &x,
                           std::vector<std::string> &names) {
    for (auto const &pair: entries_)
      if (pair.first.find(">" + x) != std::string::npos)
        names.emplace_back(pair.first);
  }
  //! Return a list of "from" maps for a mesh entity.
  void from_entity_map_names(std::string const &x,
                             std::vector<std::string> &names) {
    for (auto const &pair: entries_)
      if (pair.first.find(x + ">") != std::string::npos)
        names.emplace_back(pair.first);
  }
  //! Return a list of var names for a mesh entity.
  void entity_var_names(std::string const &x,
                        std::vector<std::string> &names) {
    for (auto const &pair: entries_)
      if (pair.first[0] == x[0])
        names.emplace_back(pair.first);
  }

public: /* These are public for testing purposes */
  Datastore *parent_; //!< The parent of this datastore (null if root)
  std::vector<dsptr> children_; //!< The list of subtrees
  void print_entries() { //!Print the datastore entries.
    for (auto const &pair: entries_) {
      std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    }
  }
};

} // namespace Ume

#endif
