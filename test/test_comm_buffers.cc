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
  \file test_comm_buffers.cc

  Tests for the pack/unpack layer that every gather and scatter runs through.
*/

#include "Ume/Comm_Buffers.hh"
#ifdef HAVE_MPI
#include "Ume/Comm_MPI.hh"
#endif
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

using namespace Ume;
using namespace Ume::Comm;

namespace {

//! Two remotes, so that the second one's offset into the buffer matters
Neighbors two_remotes() { return Neighbors{{1, {0, 2, 4}}, {3, {1, 5}}}; }

} // namespace

TEMPLATE_TEST_CASE("comm buffers: pack then unpack is the identity", "[comm]",
    DS_Types::INTV_T, DS_Types::DBLV_T, DS_Types::VEC3V_T) {
  using Field = TestType;
  using Elem = typename Field::value_type;

  Field field(8);
  for (std::size_t i = 0; i < field.size(); ++i)
    field[i] =
        Elem(static_cast<typename DS_Type_Info<Field>::base_type>(i + 1));
  Field const original = field;

  Buffers<Field> bufs{two_remotes()};
  REQUIRE(bufs.num_entries() == 5);
  bufs.pack(field);

  /* Scribble over the field so the unpack has to put the values back rather
     than merely leave them alone. */
  for (auto &e : field)
    e = Elem(0);
  // Positive control: the scribble must flip the comparison checked below.
  for (std::size_t i : {0u, 2u, 4u, 1u, 5u})
    REQUIRE_FALSE(field[i] == original[i]);
  bufs.unpack(field, Op::OVERWRITE);

  for (std::size_t i : {0u, 2u, 4u, 1u, 5u})
    CHECK(field[i] == original[i]);
}

TEST_CASE(
    "comm buffers: every component of a Vec3 gets its own slot", "[comm]") {
  /* `pack` has to advance the buffer cursor by the whole element.  When it did
     not, every Vec3 landed on top of the first one, so a buffer holding N
     points held one point's coordinates and N-1 zeros -- which is invisible
     until two entities are exchanged with the same remote. */
  DS_Types::VEC3V_T field(8, Vec3(0.0));
  field[0] = Vec3({1.0, 2.0, 3.0});
  field[2] = Vec3({4.0, 5.0, 6.0});
  field[4] = Vec3({7.0, 8.0, 9.0});

  Buffers<DS_Types::VEC3V_T> bufs{Neighbors{{1, {0, 2, 4}}}};
  bufs.pack(field);

  double const *const buf = bufs.get_buf();
  std::vector<double> const packed(buf, buf + 9);
  CHECK(packed == std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8, 9});
}

#ifdef HAVE_MPI
namespace {

//! Exchange `neighs` with this rank and check the receives match the sends
template <class Field>
void check_exchange_with_self(Comm::MPI &comm, Neighbors const &neighs) {
  using Elem = typename Field::value_type;

  Field field(8);
  for (std::size_t i = 0; i < field.size(); ++i)
    field[i] =
        Elem(static_cast<typename DS_Type_Info<Field>::base_type>(i + 1));

  Buffers<Field> sends{neighs};
  Buffers<Field> recvs{neighs};
  sends.pack(field);
  comm.exchange(sends, recvs);

  Field received(field.size(), Elem(0));
  for (auto const &n : neighs)
    for (int const e : n.elements)
      // Positive control: only the unpack can make these equal.
      CHECK_FALSE(received[e] == field[e]);
  recvs.unpack(received, Op::OVERWRITE);
  for (auto const &n : neighs)
    for (int const e : n.elements)
      CHECK(received[e] == field[e]);
}

} // namespace

/* MPI_Init can run only once per process, so this is one test case rather
   than a template test case or sections, which would construct `comm` again.
   It runs on any number of ranks: every remote is the calling rank itself. */
TEST_CASE("comm MPI: exchange with empty remotes and map virtual ranks",
    "[comm][mpi]") {
  Comm::MPI comm(nullptr, nullptr);
  int const me = comm.pe();
  // Entity::gather, scatter and gathscat skip a transport that does not.
  CHECK(comm.does_exchanges());

  /* An empty remote at the end of the list has buf_offset == buf.size(), and
     one alone has an empty buffer: in both, the remote's start address has to
     be taken without indexing the buffer. */
  for (Neighbors const &neighs :
      {Neighbors{{me, {1, 3}}, {me, {}}}, Neighbors{{me, {}}}}) {
    check_exchange_with_self<DS_Types::INTV_T>(comm, neighs);
    check_exchange_with_self<DS_Types::DBLV_T>(comm, neighs);
    check_exchange_with_self<DS_Types::VEC3V_T>(comm, neighs);
  }

  /* Every rank contributes one int to the virtual-to-real map.  On one rank
     any count fits; it takes two or more to see a count of numpe per rank
     overflow the gathered array. */
  constexpr int shift = 100;
  // Positive control: without the map, a virtual PE is taken as a real one.
  CHECK(comm.translate_pe(me + shift) == me + shift);
  comm.set_virtual_rank(me + shift);
  for (int r = 0; r < comm.numpe(); ++r)
    CHECK(comm.translate_pe(r + shift) == r);

  comm.stop();
}
#endif
