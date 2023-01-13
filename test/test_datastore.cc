#include "Ume/Datastore.hh"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
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
