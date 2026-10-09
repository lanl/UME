/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

#include "Ume/Datastore.hh"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>
#include <vector>

using dsptr = Ume::Datastore::dsptr;
using wptr = Ume::Datastore *;
using wlist = std::vector<wptr>;

void flatten(wptr curr, wlist &nodes) {
  nodes.push_back(curr);
  for (auto &c : curr->children_) {
    flatten(c.get(), nodes);
  }
}

TEST_CASE("DS ROOT", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  CHECK(root->name() == "root");
  CHECK(root->path() == "/root");
  wlist nodes;
  flatten(root.get(), nodes);
  REQUIRE(nodes.size() == 1);
}

TEST_CASE("DS ROOT+1", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  {
    wptr child1 = Ume::Datastore::create_child(root.get(), "child1");
    CHECK(child1->name() == "child1");
    CHECK(child1->path() == "/root/child1");
  }
  wlist nodes;
  flatten(root.get(), nodes);
  REQUIRE(nodes.size() == 2);
}

TEST_CASE("DS ROOT+2", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  {
    wptr child1 = Ume::Datastore::create_child(root.get(), "child1");
    CHECK(child1->name() == "child1");
    CHECK(child1->path() == "/root/child1");
    wptr child2 = Ume::Datastore::create_child(root.get(), "child2");
    CHECK(child2->name() == "child2");
    CHECK(child2->path() == "/root/child2");
  }
  wlist nodes;
  flatten(root.get(), nodes);

  REQUIRE(nodes.size() == 3);
}

TEST_CASE("DS ROOT+1+1", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  {
    wptr child1 = Ume::Datastore::create_child(root.get(), "child1");
    CHECK(child1->name() == "child1");
    CHECK(child1->path() == "/root/child1");
    wptr child2 = Ume::Datastore::create_child(child1, "child2");
    CHECK(child2->name() == "child2");
    CHECK(child2->path() == "/root/child1/child2");
  }
  wlist nodes;
  flatten(root.get(), nodes);

  REQUIRE(nodes.size() == 3);
}

TEST_CASE("DS scalar insert", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  std::unique_ptr<Ume::DS_Entry> foo = std::make_unique<Ume::DS_Entry>();
  foo->set_type(Ume::Datastore::Types::DBL);
  root->insert("some_dbl", std::move(foo));
  {
    auto &v = root->access_dbl("some_dbl");
    v = 4;
  }
  auto const &v = root->caccess_dbl("some_dbl");
  REQUIRE(v == 4);
}

class Boring : public Ume::DS_Entry {
public:
  Boring() : Ume::DS_Entry(Types::INTV) {
    auto &a = std::get<INTV_T>(data_);
    a.resize(50);
  };
  ~Boring() = default;
};

TEST_CASE("DS boring insert", "[Datastore]") {
  dsptr root = Ume::Datastore::create_root();
  root->insert("boring", std::make_unique<Boring>());
  auto const &d = root->caccess_intv("boring");
  REQUIRE(d.size() == 50);
}

/* insert refuses a key that is already present, says so, and leaves the entry
   that holds the key in place: the first insert wins. */
TEST_CASE("DS insert refuses a duplicate key", "[Datastore]") {
  using Types = Ume::Datastore::Types;
  dsptr root = Ume::Datastore::create_root();
  REQUIRE(root->insert("key", std::make_unique<Ume::DS_Entry>(Types::INT)));
  root->access_int("key") = 1;
  int const *const first = &root->caccess_int("key");

  CHECK_FALSE(root->insert("key", std::make_unique<Ume::DS_Entry>(Types::INT)));
  CHECK(&root->caccess_int("key") == first);
  CHECK(root->caccess_int("key") == 1);

  /* Positive control: a new key is accepted, and is an entry of its own. */
  CHECK(root->insert("other", std::make_unique<Ume::DS_Entry>(Types::INT)));
  CHECK(&root->caccess_int("other") != first);
  CHECK(root->caccess_int("other") == 0);
}

/* A name that a datastore does not hold is looked up in its parent, and so on
   up to the root, through both the mutable and the const accessors.  The
   nearest datastore that holds the name wins. */
TEST_CASE("DS lookup falls back to the parent chain", "[Datastore]") {
  using Types = Ume::Datastore::Types;
  dsptr root = Ume::Datastore::create_root();
  wptr child = Ume::Datastore::create_child(root.get(), "child");
  wptr grandchild = Ume::Datastore::create_child(child, "grandchild");
  REQUIRE(root->insert("shared", std::make_unique<Ume::DS_Entry>(Types::INT)));
  root->access_int("shared") = 7;
  int const *const in_root = &root->caccess_int("shared");

  CHECK(&child->access_int("shared") == in_root);
  CHECK(&child->caccess_int("shared") == in_root);
  CHECK(&grandchild->access_int("shared") == in_root);
  CHECK(&grandchild->caccess_int("shared") == in_root);
  CHECK(grandchild->caccess_int("shared") == 7);

  /* Positive control: the same name in the child shadows the root's for the
     child and the grandchild, and not for the root. */
  REQUIRE(child->insert("shared", std::make_unique<Ume::DS_Entry>(Types::INT)));
  int const *const in_child = &child->caccess_int("shared");
  CHECK(in_child != in_root);
  CHECK(&grandchild->access_int("shared") == in_child);
  CHECK(&grandchild->caccess_int("shared") == in_child);
  CHECK(&root->caccess_int("shared") == in_root);
}
