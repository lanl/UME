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
  \file test_entity_comm.cc

  Tests for Entity::gather, scatter and gathscat against a Transport that does
  not exchange.
*/

#include "Ume/Comm_Transport.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include <array>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include <type_traits>

using namespace Ume;
using namespace Ume::Comm;
using Ume::SOA_Idx::Entity;

namespace {

/*! Delivers each send buffer as the receive buffer, as if every remote were
    this rank and its copies and sources lined up one to one. */
class Loopback_Transport : public Transport {
public:
  void exchange(Buffers<DS_Types::INTV_T> const &sends,
      Buffers<DS_Types::INTV_T> &recvs) override {
    deliver(sends, recvs);
  }
  void exchange(Buffers<DS_Types::DBLV_T> const &sends,
      Buffers<DS_Types::DBLV_T> &recvs) override {
    deliver(sends, recvs);
  }
  void exchange(Buffers<DS_Types::VEC3V_T> const &sends,
      Buffers<DS_Types::VEC3V_T> &recvs) override {
    deliver(sends, recvs);
  }
  bool does_exchanges() const override { return true; }
  int stop() override { return 0; }

private:
  template <class Field>
  static void deliver(Buffers<Field> const &sends, Buffers<Field> &recvs) {
    REQUIRE(sends.buf.size() == recvs.buf.size());
    recvs.buf = sends.buf;
  }
};

/*! Implements the three exchanges and stop() but does not state whether it
    exchanges. Were does_exchanges() defaulted to false, this would compile and
    every gather, scatter and gathscat through it would be a silent no-op. */
class Unstated_Transport : public Transport {
public:
  void exchange(Buffers<DS_Types::INTV_T> const & /*sends*/,
      Buffers<DS_Types::INTV_T> & /*recvs*/) override {}
  void exchange(Buffers<DS_Types::DBLV_T> const & /*sends*/,
      Buffers<DS_Types::DBLV_T> & /*recvs*/) override {}
  void exchange(Buffers<DS_Types::VEC3V_T> const & /*sends*/,
      Buffers<DS_Types::VEC3V_T> & /*recvs*/) override {}
  int stop() override { return 0; }
};

//! Unstated_Transport with the one difference: it states the flag
class Stated_Transport : public Unstated_Transport {
public:
  bool does_exchanges() const override { return true; }
};

// Every transport states whether it exchanges; there is no default.
static_assert(std::is_abstract_v<Unstated_Transport>);
static_assert(!std::is_abstract_v<Stated_Transport>);

/*! Implements the exchange for one field type and nothing else. Transport is
    a virtual base so that Exchanges_For can combine these into one transport
    with any subset of the three exchanges. */
template <class Field> class Exchange_Of : public virtual Transport {
public:
  void exchange(
      Buffers<Field> const & /*sends*/, Buffers<Field> & /*recvs*/) override {}
};

/*! States the flag and implements stop() and the exchanges for `Fields` only.
    Were an overload defaulted to an empty body, this would compile with that
    field type left out, and a gather, scatter or gathscat of that type
    through it would unpack the untouched, zero-filled receive buffers over
    the field. */
template <class... Fields> class Exchanges_For : public Exchange_Of<Fields>... {
public:
  bool does_exchanges() const override { return true; }
  int stop() override { return 0; }
};

// Every transport implements all three exchanges; none has a default.
static_assert(
    std::is_abstract_v<Exchanges_For<DS_Types::DBLV_T, DS_Types::VEC3V_T>>);
static_assert(
    std::is_abstract_v<Exchanges_For<DS_Types::INTV_T, DS_Types::VEC3V_T>>);
static_assert(
    std::is_abstract_v<Exchanges_For<DS_Types::INTV_T, DS_Types::DBLV_T>>);
/* Positive control: each transport above differs from this one only by the
   field type it leaves out. */
static_assert(!std::is_abstract_v<
    Exchanges_For<DS_Types::INTV_T, DS_Types::DBLV_T, DS_Types::VEC3V_T>>);

enum class Kind { gather, scatter, gathscat };
constexpr std::array kinds{Kind::gather, Kind::scatter, Kind::gathscat};
constexpr std::array<std::string_view, 3> kind_names{
    "gather", "scatter", "gathscat"};

//! A field whose elements are 1, 2, ..., n, so no element is zero
template <class Field> Field numbered(std::size_t const n) {
  using Elem = typename Field::value_type;
  using Base = typename DS_Type_Info<Field>::base_type;
  Field field(n);
  for (std::size_t i = 0; i < n; ++i)
    field[i] = Elem(static_cast<Base>(i + 1));
  return field;
}

/*! Run `kind` through `comm` on a four-element entity whose element 3 is a
    copy of a source on PE 1, and whose element 0 is the source for a copy on
    PE 1.  Return the field afterwards. */
template <class Field> Field communicate(Transport &comm, Kind const kind) {
  SOA_Idx::Mesh mesh;
  mesh.comm = &comm;
  Entity &zones = mesh.zones;
  zones.resize(4, 4, 0);
  zones.myCpys = Neighbors{{1, {3}}};
  zones.mySrcs = Neighbors{{1, {0}}};

  Field field = numbered<Field>(4);
  switch (kind) {
  case Kind::gather:
    zones.gather(Op::OVERWRITE, field);
    break;
  case Kind::scatter:
    zones.scatter(field);
    break;
  case Kind::gathscat:
    zones.gathscat(Op::OVERWRITE, field);
    break;
  }
  return field;
}

} // namespace

TEMPLATE_TEST_CASE("entity comm: a transport that does not exchange leaves "
                   "the field alone",
    "[comm]", DS_Types::INTV_T, DS_Types::DBLV_T, DS_Types::VEC3V_T) {
  using Field = TestType;
  /* Dummy_Transport's exchange is a no-op, so a scatter used to unpack a
     zero-filled receive buffer over every copy, and a gather the same over
     every source. */
  Dummy_Transport dummy;
  Loopback_Transport loopback;
  Field const original = numbered<Field>(4);

  for (std::size_t k = 0; k < kinds.size(); ++k) {
    CAPTURE(kind_names[k]);
    CHECK(communicate<Field>(dummy, kinds[k]) == original);
    /* Positive control: a transport that does exchange moves a value between
       elements 0 and 3, so the same call must change the field. */
    CHECK_FALSE(communicate<Field>(loopback, kinds[k]) == original);
  }
}
