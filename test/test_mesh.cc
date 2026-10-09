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
  \file test_mesh.cc

  Tests for the invariants of SOA_Idx::Mesh itself, as distinct from any
  particular mesh it might hold.
*/

#include "Ume/Mesh_Base.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/face_area.hh"
#include "Ume/gradient.hh"
#include "Ume/mem_exec_spaces.hh"
#include "Ume/renumbering.hh"
#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <istream>
#include <memory>
#include <new>
#include <numeric>
#include <random>
#include <ranges>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using Ume::SOA_Idx::Mesh;

namespace {

/* Neither copyable nor movable, by construction or by assignment. */
template <class T>
constexpr bool is_pinned_v =
    !std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T> &&
    !std::is_move_constructible_v<T> && !std::is_move_assignable_v<T>;

/* The object representation of `value`, for a member that may hold bytes
   that are not a valid value of its type. */
template <class T> std::array<std::byte, sizeof(T)> bytes_of(T const &value) {
  std::array<std::byte, sizeof(T)> bytes;
  std::memcpy(bytes.data(), &value, sizeof(T));
  return bytes;
}

} // namespace

/* Every Entity caches the `Mesh *` it was constructed against, and a Mesh
   hands each of its Entity members `this` in its constructor.  Relocating
   either leaves that pointer aimed at the original -- `auto z = mesh.zones;`
   would resolve ds() and comm() through the mesh it was copied from -- and
   nothing in the Entity API can detect it, so neither type can be copied or
   moved.  Entity itself is abstract, which makes it trivially not
   constructible; the concrete entities are the ones a caller could copy. */
static_assert(is_pinned_v<Mesh>);
static_assert(!std::is_copy_assignable_v<Ume::SOA_Idx::Entity>);
static_assert(!std::is_move_assignable_v<Ume::SOA_Idx::Entity>);
static_assert(is_pinned_v<Ume::SOA_Idx::Corners>);
static_assert(is_pinned_v<Ume::SOA_Idx::Edges>);
static_assert(is_pinned_v<Ume::SOA_Idx::Faces>);
static_assert(is_pinned_v<Ume::SOA_Idx::Points>);
static_assert(is_pinned_v<Ume::SOA_Idx::Sides>);
static_assert(is_pinned_v<Ume::SOA_Idx::Zones>);
static_assert(is_pinned_v<Ume::SOA_Idx::Iotas>);
/* Positive control: a movable, copyable type is not pinned. */
static_assert(!is_pinned_v<std::vector<int>>);

/* Consequently a Mesh cannot be an element of a std::vector that grows,
   reallocates or sorts itself: push_back, resize, reserve and sort all
   require MoveInsertable or move assignment.  A vector built at its final
   size, `std::vector<Mesh> v(3)`, needs only DefaultInsertable and compiles.
   No type trait states that -- vector's members are declared for every
   element type and only fail when their bodies are instantiated -- so the
   guarantee is the assertions above, and the compile error lands at the point
   of use. */

TEST_CASE("mesh: entities point back at their own mesh", "[mesh]") {
  Mesh mesh;
  CHECK(&mesh.corners.mesh() == &mesh);
  CHECK(&mesh.edges.mesh() == &mesh);
  CHECK(&mesh.faces.mesh() == &mesh);
  CHECK(&mesh.points.mesh() == &mesh);
  CHECK(&mesh.sides.mesh() == &mesh);
  CHECK(&mesh.zones.mesh() == &mesh);
  CHECK(&mesh.iotas.mesh() == &mesh);

  /* Positive control: another mesh's entities point at that mesh, not this
     one, so the comparison distinguishes meshes. */
  Mesh other;
  CHECK(&other.zones.mesh() != &mesh);
}

TEST_CASE("mesh: entity ds() and comm() are the owning mesh's", "[mesh]") {
  /* Entity::ds() and comm() reach Mesh_Base through the Mesh back-pointer;
     the conversion has to land on the Mesh_Base subobject of this mesh. */
  Mesh mesh;
  Ume::Comm::Dummy_Transport transport;
  mesh.comm = &transport;
  Ume::SOA_Idx::Entity const *const entities[] = {&mesh.corners, &mesh.edges,
      &mesh.faces, &mesh.points, &mesh.sides, &mesh.zones, &mesh.iotas};
  for (Ume::SOA_Idx::Entity const *e : entities) {
    CHECK(&e->ds() == mesh.ds.get());
    CHECK(&e->comm() == &transport);
  }
  /* The non-const overloads resolve the same way. */
  CHECK(&mesh.zones.ds() == mesh.ds.get());
  CHECK(&mesh.zones.comm() == &transport);

  /* Positive control: another mesh, with its own Datastore and Transport, is
     not this one, so the comparisons distinguish meshes. */
  Mesh other;
  Ume::Comm::Dummy_Transport other_transport;
  other.comm = &other_transport;
  CHECK(&other.zones.ds() != mesh.ds.get());
  CHECK(&other.zones.comm() != &transport);
  CHECK(&other.zones.ds() == other.ds.get());
}

TEST_CASE(
    "mesh: sorting meshes held by pointer keeps them addressable", "[mesh]") {
  /* The shape of read_meshes() in ume_serial.cc: meshes arriving out of rank
     order are sorted into place.  Permuting the pointers has to leave every
     Entity back-pointer valid. */
  std::vector<std::unique_ptr<Mesh>> ranks;
  for (int const pe : {2, 0, 1}) {
    ranks.push_back(std::make_unique<Mesh>());
    ranks.back()->mype = pe;
  }
  std::vector<Mesh const *> before;
  for (auto const &m : ranks)
    before.push_back(m.get());

  std::ranges::sort(ranks, {}, [](auto const &m) { return m->mype; });

  REQUIRE(ranks.size() == 3);
  for (std::size_t i = 0; i < ranks.size(); ++i) {
    CHECK(ranks[i]->mype == static_cast<int>(i));
    /* Still self-consistent wherever it sits in the vector. */
    CHECK(&ranks[i]->points.mesh() == ranks[i].get());
  }
  /* The set of addresses is unchanged: the sort moved pointers, not meshes. */
  std::vector<Mesh const *> after;
  for (auto const &m : ranks)
    after.push_back(m.get());
  CHECK(after != before);
  std::ranges::sort(before);
  std::ranges::sort(after);
  CHECK(after == before);
}

/* A partition can legitimately come out empty -- ask for more ranks than the
   mesh has zones and some of them get nothing -- so the derived variables have
   to survive a mesh with no entities in it.  They take the address of element
   zero of the arrays they read, which is a hard error on an empty one. */
TEST_CASE("an empty mesh can still derive its variables", "[mesh]") {
  Mesh mesh;
  Ume::Comm::Dummy_Transport transport;
  mesh.comm = &transport;
  mesh.ivtag = UME_VERSION_2;
  mesh.version_header = true;
  mesh.mype = 0;
  mesh.numpe = 1;
  mesh.geo = Mesh::CARTESIAN;
  mesh.dump_iotas = false;
  Ume::SOA_Idx::Entity *const entities[] = {&mesh.points, &mesh.edges,
      &mesh.faces, &mesh.zones, &mesh.corners, &mesh.sides, &mesh.iotas};
  for (Ume::SOA_Idx::Entity *e : entities)
    e->resize(0, 0, 0);

  for (char const *name : {"zcoord", "fcoord", "ecoord", "side_surf",
           "side_surz", "corner_csurf", "point_norm"})
    CHECK(mesh.ds->caccess_vec3v(name).empty());
  for (char const *name : {"side_vol", "corner_vol"})
    CHECK(mesh.ds->caccess_dblv(name).empty());
  /* The ragged derivations index their accumulators with `at()`, so all they
     have to do here is not throw -- there are no rows to look at. */
  for (char const *name :
      {"m:z>c", "m:z>p", "m:z>pz", "m:c>s", "m:p>zs", "m:p>rc"})
    CHECK_NOTHROW(mesh.ds->caccess_intrr(name));

  /* The kernels that consume those variables build the same kind of View over
     their own arrays, so they have to survive the empty mesh too.  The gradient
     outputs start non-empty so that the size checks see the kernels resize
     them. */
  Ume::DS_Types::DBLV_T const zone_field;
  Ume::DS_Types::VEC3V_T zone_gradient{Ume::DS_Types::VEC3_T(1.0)};
  Ume::DS_Types::VEC3V_T point_gradient{Ume::DS_Types::VEC3_T(1.0)};
  Ume::gradzatz(mesh, zone_field, zone_gradient, point_gradient);
  CHECK(zone_gradient.empty());
  CHECK(point_gradient.empty());

  zone_gradient.assign(1, Ume::DS_Types::VEC3_T(1.0));
  point_gradient.assign(1, Ume::DS_Types::VEC3_T(1.0));
  Ume::gradzatz_invert(mesh, zone_field, zone_gradient, point_gradient);
  CHECK(zone_gradient.empty());
  CHECK(point_gradient.empty());

  /* calc_face_area fills a caller-sized output, so there is nothing to check
     beyond surviving the call. */
  Ume::DS_Types::DBLV_T face_area;
  Ume::calc_face_area(mesh, face_area);
}

namespace {

/* The number of faces whose area is not `reference` to rounding.  Concurrent
   atomic adds may sum a face's sides in any order, so the comparison is not
   exact. */
std::size_t count_wrong_areas(Ume::DS_Types::DBLV_T const &face_area,
    Ume::DS_Types::DBLV_T const &reference) {
  std::size_t wrong = 0;
  for (std::size_t f = 0; f < reference.size(); ++f)
    if (!(std::abs(face_area[f] - reference[f]) <= 1e-12 * reference[f]))
      ++wrong;
  return wrong;
}

} // namespace

/* calc_face_area walks the sides, and each face is covered twice, once by the
   sides of each zone it separates; a side and its partner `m:s>s2` are the
   same triangle seen from either zone, so the face area takes one of the two.
   On a threaded backend the two members of a pair can run concurrently, and
   the choice must not depend on which got there first.  The mesh here is
   disjoint triangular faces, three sides a zone, with the side numbering
   shuffled so that the members of a pair land at arbitrary distances on
   arbitrary threads, and the call is repeated because a race need not show
   on any one run.  Every fourth face is a non-source copy of a face owned
   elsewhere, which does not count its sides here.  Every eighth is on the
   problem boundary, its partner sides not real (`mask < 1`), and every eighth
   has sides that are their own partners, which the side tag the kernel used
   to keep counted once. */
TEST_CASE("face areas take one side of each pair, whichever thread runs it",
    "[mesh][face_area]") {
  constexpr int nface = 200'000;
  constexpr int sides_per_zone = 3;
  constexpr int npoint = sides_per_zone * nface;
  constexpr int nside = 2 * sides_per_zone * nface;
  constexpr int repetitions = 20;

  Mesh mesh;
  Ume::Comm::Dummy_Transport transport;
  mesh.comm = &transport;
  mesh.ivtag = UME_VERSION_2;
  mesh.version_header = true;
  mesh.mype = 0;
  mesh.numpe = 1;
  mesh.geo = Mesh::CARTESIAN;
  mesh.dump_iotas = false;
  Ume::SOA_Idx::Entity *const entities[] = {&mesh.points, &mesh.edges,
      &mesh.faces, &mesh.zones, &mesh.corners, &mesh.sides, &mesh.iotas};
  for (Ume::SOA_Idx::Entity *e : entities)
    e->resize(0, 0, 0);
  mesh.points.resize(npoint, npoint, 0);
  mesh.faces.resize(nface, nface, 0);
  mesh.sides.resize(nside, nside, 0);

  auto const is_copy = [](int const f) { return f % 4 == 3; };
  auto const is_boundary = [](int const f) { return f % 8 == 1; };
  auto const is_self_paired = [](int const f) { return f % 8 == 5; };

  /* Face f is the triangle on points 3f, 3f+1, 3f+2, of area (1 + f % 3)/2. */
  auto &pcoord = mesh.ds->access_vec3v("pcoord");
  for (int f = 0; f < nface; ++f) {
    double const z = f;
    double const base = 1.0 + f % 3;
    pcoord[3 * f + 0] = Ume::Vec3(std::array{0.0, 0.0, z});
    pcoord[3 * f + 1] = Ume::Vec3(std::array{base, 0.0, z});
    pcoord[3 * f + 2] = Ume::Vec3(std::array{0.0, 1.0, z});
  }

  std::fill(mesh.faces.mask.begin(), mesh.faces.mask.end(), short{1});
  for (int f = 0; f < nface; ++f)
    mesh.faces.comm_type[f] = is_copy(f) ? Ume::SOA_Idx::Entity::COPY
                                         : Ume::SOA_Idx::Entity::INTERNAL;

  /* Slot k < 3 of face f is the side on edge {k, k+1} in the first zone; slot
     k + 3 is the same edge, reversed, in the second. */
  std::vector<int> side_of_slot(nside);
  std::iota(side_of_slot.begin(), side_of_slot.end(), 0);
  std::shuffle(side_of_slot.begin(), side_of_slot.end(), std::mt19937{8});
  auto &s_to_f = mesh.ds->access_intv("m:s>f");
  auto &s_to_p1 = mesh.ds->access_intv("m:s>p1");
  auto &s_to_p2 = mesh.ds->access_intv("m:s>p2");
  auto &s_to_s2 = mesh.ds->access_intv("m:s>s2");
  for (int f = 0; f < nface; ++f) {
    for (int k = 0; k < sides_per_zone; ++k) {
      int const s = side_of_slot[2 * sides_per_zone * f + k];
      int const s2 = side_of_slot[2 * sides_per_zone * f + sides_per_zone + k];
      int const a = sides_per_zone * f + k;
      int const b = sides_per_zone * f + (k + 1) % sides_per_zone;
      s_to_f[s] = s_to_f[s2] = f;
      s_to_p1[s] = s_to_p2[s2] = a;
      s_to_p2[s] = s_to_p1[s2] = b;
      s_to_s2[s] = is_self_paired(f) ? s : s2;
      s_to_s2[s2] = is_self_paired(f) ? s2 : s;
      mesh.sides.mask[s] = 1;
      mesh.sides.mask[s2] = is_self_paired(f) ? short{0}
          : is_boundary(f)                    ? short{-1}
                                              : short{1};
    }
  }

  /* The single-threaded reference: the first zone's sides of each face that
     this rank owns.  It reads the same side_surz the kernel does, so it holds
     whatever the face centers came out as. */
  auto const &surz = mesh.ds->caccess_vec3v("side_surz");
  Ume::DS_Types::DBLV_T reference(nface, 0.0);
  int degenerate = 0;
  for (int f = 0; f < nface; ++f) {
    if (is_copy(f))
      continue;
    for (int k = 0; k < sides_per_zone; ++k)
      reference[f] +=
          Ume::vectormag(surz[side_of_slot[2 * sides_per_zone * f + k]]);
    if (!(reference[f] > 0.0))
      ++degenerate;
  }
  REQUIRE(degenerate == 0);

  /* Positive control: one face counted twice is one wrong face. */
  Ume::DS_Types::DBLV_T doubled = reference;
  doubled[0] *= 2.0;
  REQUIRE(count_wrong_areas(doubled, reference) == 1);

  Ume::DS_Types::DBLV_T face_area(nface);
  for (int rep = 0; rep < repetitions; ++rep) {
    std::fill(face_area.begin(), face_area.end(), -1.0);
    Ume::calc_face_area(mesh, face_area);
    INFO("repetition " << rep);
    CHECK(count_wrong_areas(face_area, reference) == 0);
  }
}

namespace {

/* The number of entries of `centroid` that differ from `reference`.  The
   coordinates are small integers, so every partial sum is exact and the
   centroids are equal bit for bit whatever order the points were added in. */
std::size_t count_wrong_centroids(Ume::DS_Types::VEC3V_T const &centroid,
    Ume::DS_Types::VEC3V_T const &reference) {
  std::size_t wrong = 0;
  for (std::size_t i = 0; i < reference.size(); ++i)
    if (!(centroid[i] == reference[i]))
      ++wrong;
  return wrong;
}

/* The centroid of the points `elem_to_pt` hangs on each of `nelem` elements,
   over the members whose `mask` is set, summed in one thread. */
Ume::DS_Types::VEC3V_T serial_centroids(int const nelem,
    std::vector<int> const &member_to_elem,
    std::vector<int> const &member_to_pt, std::vector<short> const &mask,
    Ume::DS_Types::VEC3V_T const &pcoord) {
  Ume::DS_Types::VEC3V_T centroid(nelem, Ume::Vec3(0.0));
  std::vector<int> count(nelem, 0);
  for (std::size_t m = 0; m < member_to_elem.size(); ++m) {
    if (mask[m] == 0)
      continue;
    centroid[member_to_elem[m]] += pcoord[member_to_pt[m]];
    ++count[member_to_elem[m]];
  }
  for (int e = 0; e < nelem; ++e)
    centroid[e] /= static_cast<double>(count[e]);
  return centroid;
}

} // namespace

/* zcoord and fcoord sum the points of each zone (face) over its corners
   (sides) in a parallel loop, and divide by how many there were.  With a
   threaded host backend, two corners of one zone counted at once must both be
   counted.  The mesh here has few zones and faces and many corners and sides
   on each, numbered round-robin so that every thread's block of the loop
   touches every zone, and every sixteenth corner and side is masked out.  The
   derived variables are cached once computed, so each repetition builds a
   fresh mesh. */
TEST_CASE("zone and face centroids count every point, whichever thread "
          "counts it",
    "[mesh][centroid]") {
  constexpr int nelem = 64;
  constexpr int members_per_elem = 8192;
  constexpr int nmember = nelem * members_per_elem;
  constexpr int repetitions = 10;
  INFO("host concurrency " << HostExecSpace().concurrency());

  auto const elem_of = [](int const m) { return m % nelem; };
  auto const is_masked = [](int const m) { return m / nelem % 16 == 15; };

  for (int rep = 0; rep < repetitions; ++rep) {
    Ume::Comm::Dummy_Transport transport;
    Mesh mesh;
    mesh.comm = &transport;
    mesh.ivtag = UME_VERSION_2;
    mesh.version_header = true;
    mesh.mype = 0;
    mesh.numpe = 1;
    mesh.geo = Mesh::CARTESIAN;
    mesh.dump_iotas = false;
    Ume::SOA_Idx::Entity *const entities[] = {&mesh.points, &mesh.edges,
        &mesh.faces, &mesh.zones, &mesh.corners, &mesh.sides, &mesh.iotas};
    for (Ume::SOA_Idx::Entity *e : entities)
      e->resize(0, 0, 0);
    mesh.points.resize(nmember, nmember, 0);
    mesh.zones.resize(nelem, nelem, 0);
    mesh.faces.resize(nelem, nelem, 0);
    mesh.corners.resize(nmember, nmember, 0);
    mesh.sides.resize(nmember, nmember, 0);

    /* Point p hangs on corner p and side p. */
    auto &pcoord = mesh.ds->access_vec3v("pcoord");
    for (int p = 0; p < nmember; ++p)
      pcoord[p] = Ume::Vec3(std::array{static_cast<double>(p % 5),
          static_cast<double>(p % 7), static_cast<double>(p % 11)});
    std::fill(mesh.zones.mask.begin(), mesh.zones.mask.end(), short{1});
    std::fill(mesh.faces.mask.begin(), mesh.faces.mask.end(), short{1});

    auto &c_to_z = mesh.ds->access_intv("m:c>z");
    auto &c_to_p = mesh.ds->access_intv("m:c>p");
    auto &s_to_f = mesh.ds->access_intv("m:s>f");
    auto &s_to_p1 = mesh.ds->access_intv("m:s>p1");
    for (int m = 0; m < nmember; ++m) {
      c_to_z[m] = s_to_f[m] = elem_of(m);
      c_to_p[m] = s_to_p1[m] = m;
      mesh.corners.mask[m] = mesh.sides.mask[m] =
          is_masked(m) ? short{0} : short{1};
    }

    Ume::DS_Types::VEC3V_T const zone_reference =
        serial_centroids(nelem, c_to_z, c_to_p, mesh.corners.mask, pcoord);
    Ume::DS_Types::VEC3V_T const face_reference =
        serial_centroids(nelem, s_to_f, s_to_p1, mesh.sides.mask, pcoord);

    if (rep == 0) {
      /* Positive control: point 0 is the origin, so dropping corner 0 leaves
         zone 0's sum as it was and its count one short, which is what a
         lost increment does.  That is one wrong centroid. */
      std::vector<short> one_lost = mesh.corners.mask;
      one_lost[0] = 0;
      REQUIRE(count_wrong_centroids(
                  serial_centroids(nelem, c_to_z, c_to_p, one_lost, pcoord),
                  zone_reference) == 1);
    }

    INFO("repetition " << rep);
    auto const &zcoord = mesh.ds->caccess_vec3v("zcoord");
    REQUIRE(zcoord.size() == zone_reference.size());
    CHECK(count_wrong_centroids(zcoord, zone_reference) == 0);
    auto const &fcoord = mesh.ds->caccess_vec3v("fcoord");
    REQUIRE(fcoord.size() == face_reference.size());
    CHECK(count_wrong_centroids(fcoord, face_reference) == 0);
  }
}

namespace {

/* The indices `range` yields, in order. */
std::vector<int> indices_of(std::ranges::common_range auto const &range) {
  return {std::ranges::begin(range), std::ranges::end(range)};
}

/* A mesh with no entities in it, for the index range tests to resize one
   entity of. */
struct Bare_Mesh {
  Ume::Comm::Dummy_Transport transport;
  Mesh mesh;
  Bare_Mesh() {
    mesh.comm = &transport;
    mesh.ivtag = UME_VERSION_2;
    mesh.version_header = true;
    mesh.mype = 0;
    mesh.numpe = 1;
    mesh.geo = Mesh::CARTESIAN;
    mesh.dump_iotas = false;
    Ume::SOA_Idx::Entity *const entities[] = {&mesh.points, &mesh.edges,
        &mesh.faces, &mesh.zones, &mesh.corners, &mesh.sides, &mesh.iotas};
    for (Ume::SOA_Idx::Entity *e : entities)
      e->resize(0, 0, 0);
  }
};

} // namespace

/* The index ranges are half-open: every element of an Entity is in exactly
   one of local_indices() and ghost_indices(), and all_indices() is their
   concatenation.  The sizes are checked before the contents, so that a range
   whose bound lies below its start fails rather than iterates. */
TEST_CASE("entity index ranges cover every element", "[mesh][entity]") {
  Bare_Mesh bare;
  auto &points = bare.mesh.points;
  points.resize(5, 8, 3);

  std::vector<int> const every_point{0, 1, 2, 3, 4, 5, 6, 7};
  REQUIRE(std::ranges::size(points.all_indices()) == 8);
  CHECK(indices_of(points.all_indices()) == every_point);
  REQUIRE(std::ranges::size(points.local_indices()) == 5);
  CHECK(indices_of(points.local_indices()) == std::vector<int>{0, 1, 2, 3, 4});
  REQUIRE(std::ranges::size(points.ghost_indices()) == 3);
  CHECK(indices_of(points.ghost_indices()) == std::vector<int>{5, 6, 7});
  CHECK(points.ghost_size() == 3);
  REQUIRE(std::ranges::size(points.ghost_indices_offset()) == 3);
  CHECK(indices_of(points.ghost_indices_offset()) == std::vector<int>{0, 1, 2});

  /* Positive control: the closed-interval bound the ranges used to have,
     `size() - 1`, drops the last element, and the comparison sees it. */
  CHECK(
      indices_of(std::ranges::iota_view{0, points.size() - 1}) != every_point);
}

/* An empty Entity -- an empty rank's, say -- yields no indices at all. */
TEST_CASE(
    "entity index ranges are empty on an empty entity", "[mesh][entity]") {
  Bare_Mesh bare;
  auto &points = bare.mesh.points;
  REQUIRE(points.size() == 0);

  CHECK(std::ranges::size(points.all_indices()) == 0);
  CHECK(points.all_indices().empty());
  CHECK(std::ranges::size(points.local_indices()) == 0);
  CHECK(points.local_indices().empty());
  CHECK(std::ranges::size(points.ghost_indices()) == 0);
  CHECK(points.ghost_indices().empty());
  CHECK(points.ghost_size() == 0);
  CHECK(std::ranges::size(points.ghost_indices_offset()) == 0);
  CHECK(points.ghost_indices_offset().empty());

  /* Positive control: one local and one ghost element make every range
     non-empty, so the checks above see the entity's size. */
  points.resize(1, 2, 1);
  CHECK(indices_of(points.all_indices()) == std::vector<int>{0, 1});
  CHECK(indices_of(points.local_indices()) == std::vector<int>{0});
  CHECK(indices_of(points.ghost_indices()) == std::vector<int>{1});
  CHECK(points.ghost_size() == 1);
  CHECK(indices_of(points.ghost_indices_offset()) == std::vector<int>{0});
}

/* An Entity with no ghosts has no ghost indices, offset or not; a loop over
   ghost_indices_offset() indexing the ghost arrays (sized by resize()'s ghost
   argument, here zero) must not run. */
TEST_CASE(
    "entity ghost ranges are empty on a ghost-free entity", "[mesh][entity]") {
  Bare_Mesh bare;
  auto &points = bare.mesh.points;
  points.resize(4, 4, 0);
  REQUIRE(points.ghost_mask.empty());

  REQUIRE(std::ranges::size(points.all_indices()) == 4);
  CHECK(indices_of(points.all_indices()) == std::vector<int>{0, 1, 2, 3});
  CHECK(indices_of(points.local_indices()) == std::vector<int>{0, 1, 2, 3});
  CHECK(points.ghost_indices().empty());
  CHECK(points.ghost_size() == 0);
  CHECK(std::ranges::size(points.ghost_indices_offset()) == 0);
  CHECK(points.ghost_indices_offset().empty());

  /* Positive control: a single ghost makes both ghost ranges non-empty. */
  points.resize(4, 5, 1);
  CHECK(indices_of(points.ghost_indices()) == std::vector<int>{4});
  CHECK(points.ghost_size() == 1);
  CHECK(indices_of(points.ghost_indices_offset()) == std::vector<int>{0});
}

namespace {

/* Whether `numbers` holds each of 0, 1, ..., size() - 1 exactly once. */
bool is_permutation_of_first(std::vector<int> numbers) {
  std::ranges::sort(numbers);
  return std::ranges::equal(
      numbers, std::views::iota(0, static_cast<int>(numbers.size())));
}

} // namespace

/* new_numbering is a counting sort: it numbers the live x in the order of the
   y each one hangs on, and the x on one y consecutively, in their own order.
   Seven sides hang on three points here, and side 5 is null: the six live
   sides must get each of 0, ..., 5 once, and the null one keeps its value. */
TEST_CASE("new_numbering numbers the live entities as a permutation",
    "[mesh][renumbering]") {
  Bare_Mesh bare;
  auto &x = bare.mesh.sides;
  auto const &y = bare.mesh.points;
  x.resize(7, 7, 0);
  bare.mesh.points.resize(3, 3, 0);
  std::ranges::fill(x.mask, short{1});
  x.mask[5] = 0;
  Ume::DS_Types::INTV_T const x_to_y{2, 0, 1, 0, 2, 2, 1};

  int x_max = 0;
  Ume::DS_Types::INTV_T x_to_xnew(7, -1);
  Ume::new_numbering(x, y, x_to_y, x_max, x_to_xnew);

  /* Point 0 holds sides 1 and 3, point 1 sides 2 and 6, point 2 sides 0
     and 4. */
  CHECK(x_to_xnew == Ume::DS_Types::INTV_T{4, 0, 2, 1, 5, -1, 3});
  CHECK(x_max == 5);
  std::vector<int> live;
  for (int s : x.local_indices())
    if (x.mask[s] != 0)
      live.push_back(x_to_xnew[s]);
  CHECK(is_permutation_of_first(live));

  /* Positive control: giving every x on one y the same number, as
     new_numbering used to, is not a permutation. */
  CHECK_FALSE(is_permutation_of_first({4, 0, 2, 0, 4, 2}));
}

/* An empty rank has nothing to number, and numbering it must not fail. */
TEST_CASE(
    "new_numbering numbers nothing on an empty rank", "[mesh][renumbering]") {
  Bare_Mesh bare;
  auto &x = bare.mesh.sides;
  auto const &y = bare.mesh.points;
  REQUIRE(x.size() == 0);
  REQUIRE(y.size() == 0);

  int x_max = -1;
  Ume::DS_Types::INTV_T x_to_xnew;
  Ume::new_numbering(x, y, Ume::DS_Types::INTV_T{}, x_max, x_to_xnew);
  CHECK(x_max == -1);

  /* Positive control: one live x on one y is numbered 0, which moves
     x_max. */
  x.resize(1, 1, 0);
  bare.mesh.points.resize(1, 1, 0);
  x.mask[0] = 1;
  x_to_xnew.assign(1, -1);
  Ume::new_numbering(x, y, Ume::DS_Types::INTV_T{0}, x_max, x_to_xnew);
  CHECK(x_to_xnew == Ume::DS_Types::INTV_T{0});
  CHECK(x_max == 0);
}

/* The point wavefront starts again once per disjoint piece of a rank's mesh,
   from the unnumbered point with the most sides.  Here the pieces are a fan,
   whose hub, point 3, has the most sides and seeds first, and a triangle on
   points 0, 1, 2, whose best seed is point 0, the lowest-numbered of three
   equals.  Every point must get a number, each of 0, ..., 6 once. */
TEST_CASE("renumber_p numbers every disjoint piece of the mesh",
    "[mesh][renumbering]") {
  constexpr std::array<std::array<int, 2>, 7> side_points{
      {{0, 1}, {1, 2}, {2, 0}, {3, 4}, {3, 5}, {3, 6}, {4, 5}}};
  constexpr int npoint = 7;
  constexpr int nside = static_cast<int>(side_points.size());
  constexpr int niota = 2 * nside;

  Bare_Mesh bare;
  Mesh &mesh = bare.mesh;
  mesh.points.resize(npoint, npoint, 0);
  mesh.sides.resize(nside, nside, 0);
  mesh.iotas.resize(niota, niota, 0);
  std::ranges::fill(mesh.points.mask, short{1});
  std::ranges::fill(mesh.sides.mask, short{1});
  std::ranges::fill(mesh.iotas.mask, short{1});

  /* Iota 2s is side s at its first point, iota 2s + 1 at its second. */
  auto &s_to_p1 = mesh.ds->access_intv("m:s>p1");
  auto &s_to_p2 = mesh.ds->access_intv("m:s>p2");
  auto &a_to_p = mesh.ds->access_intv("m:a>p");
  auto &a_to_s = mesh.ds->access_intv("m:a>s");
  for (int s = 0; s < nside; ++s) {
    s_to_p1[s] = a_to_p[2 * s] = side_points[s][0];
    s_to_p2[s] = a_to_p[2 * s + 1] = side_points[s][1];
    a_to_s[2 * s] = a_to_s[2 * s + 1] = s;
  }

  Ume::DS_Types::INTV_T const p_to_pnew = Ume::renumber_p(mesh);
  REQUIRE(p_to_pnew.size() == npoint);
  CAPTURE(p_to_pnew);
  CHECK(is_permutation_of_first(p_to_pnew));

  /* Positive control: the triangle left unnumbered is not a permutation. */
  Ume::DS_Types::INTV_T unreached = p_to_pnew;
  for (int p : {0, 1, 2})
    unreached[p] = -1;
  CHECK_FALSE(is_permutation_of_first(unreached));
}

namespace {

/* Every Entity member of `mesh`, named, in declaration order. */
std::array<std::pair<char const *, Ume::SOA_Idx::Entity *>, 7> entities_of(
    Mesh &mesh) {
  return {
      {{"points", &mesh.points}, {"edges", &mesh.edges}, {"faces", &mesh.faces},
          {"sides", &mesh.sides}, {"corners", &mesh.corners},
          {"zones", &mesh.zones}, {"iotas", &mesh.iotas}}};
}

} // namespace

/* Mesh::operator== is what a write/read round trip is checked with, so a
   difference in any one entity has to make two meshes unequal.  Each entity in
   turn is given a mask entry the other mesh does not have, and then the face
   maps, which only Faces compares, are given an entry each. */
TEST_CASE("mesh: equality compares every entity", "[mesh]") {
  Bare_Mesh lhs;
  Bare_Mesh rhs;
  for (Mesh *const m : {&lhs.mesh, &rhs.mesh}) {
    for (auto const &[name, e] : entities_of(*m)) {
      e->resize(1, 1, 0);
      e->mask[0] = 1;
    }
    /* A default-constructed Vec3 is uninitialized. */
    m->ds->access_vec3v("pcoord")[0] = Ume::Vec3(0.0);
  }
  REQUIRE(lhs.mesh == rhs.mesh);

  for (auto const &[name, e] : entities_of(rhs.mesh)) {
    INFO("entity " << name);
    e->mask[0] = 0;
    CHECK_FALSE(lhs.mesh == rhs.mesh);
    e->mask[0] = 1;
    REQUIRE(lhs.mesh == rhs.mesh);
  }

  for (char const *const map : {"m:f>z1", "m:f>z2"}) {
    INFO("map " << map);
    auto &face_to_zone = rhs.mesh.ds->access_intv(map);
    REQUIRE(face_to_zone.size() == 1);
    face_to_zone[0] = 1;
    CHECK_FALSE(lhs.mesh == rhs.mesh);
    face_to_zone[0] = 0;
    REQUIRE(lhs.mesh == rhs.mesh);
  }
}

namespace {

/* `n` values of type T, each one more than the last, starting at `next`, which
   is left one past the last. */
template <class T>
std::vector<T> distinct_values(int &next, std::size_t const n) {
  std::vector<T> values(n);
  for (T &v : values)
    v = static_cast<T>(next++);
  return values;
}

/* Give every field that Mesh::write emits a value no other field has: the mesh
   scalars, the ten Entity fields of each entity, and each entity's maps.  A
   field that is not written, not read, or read into another field's slot then
   shows as a difference after a round trip.  With `with_iotas` false the
   iotas are left empty, as an input without iotas reads them. */
void populate(Mesh &mesh, bool const with_iotas) {
  mesh.ivtag = UME_VERSION_2;
  mesh.mype = 2;
  mesh.numpe = 5;
  mesh.geo = Mesh::CYLINDRICAL;
  mesh.dump_iotas = with_iotas;

  constexpr int local = 3;
  constexpr int total = 5;
  constexpr int ghost = total - local;
  int next = 1;
  for (auto const &[name, e] : entities_of(mesh)) {
    if (e == &mesh.iotas && !with_iotas)
      continue;
    e->resize(local, total, ghost);
    e->mask = distinct_values<short>(next, total);
    e->comm_type = distinct_values<int>(next, total);
    e->cpy_idx = distinct_values<int>(next, ghost);
    e->src_pe = distinct_values<int>(next, ghost);
    e->src_idx = distinct_values<int>(next, ghost);
    e->ghost_mask = distinct_values<int>(next, ghost);
    /* Braced initializers are evaluated left to right. */
    e->myCpys = {{next++, distinct_values<int>(next, 2)},
        {next++, distinct_values<int>(next, 1)}};
    e->mySrcs = {{next++, distinct_values<int>(next, 3)}};
    e->subsets = {
        {std::string{name} + "_a", next++, distinct_values<int>(next, 2),
            distinct_values<short>(next, 2)},
        {std::string{name} + "_b", next++, distinct_values<int>(next, 1),
            distinct_values<short>(next, 1)}};
  }

  /* Points::resize leaves the Vec3s uninitialized. */
  for (auto &x : mesh.ds->access_vec3v("pcoord")) {
    x = Ume::Vec3(std::array{static_cast<double>(next),
        static_cast<double>(next + 1), static_cast<double>(next + 2)});
    next += 3;
  }
  std::vector<char const *> maps{"m:e>p1", "m:e>p2", "m:f>z1", "m:f>z2",
      "m:s>z", "m:s>p1", "m:s>p2", "m:s>e", "m:s>f", "m:s>c1", "m:s>c2",
      "m:s>s2", "m:s>s3", "m:s>s4", "m:s>s5", "m:c>p", "m:c>z"};
  if (with_iotas)
    maps.insert(maps.end(), {"m:a>z", "m:a>f", "m:a>p", "m:a>e", "m:a>s"});
  for (char const *const map : maps)
    mesh.ds->access_intv(map) = distinct_values<int>(next, total);
}

/* Whether `is` has been read to its end. */
bool exhausted(std::istream &is) {
  return is.peek() == std::istream::traits_type::eof();
}

} // namespace

/* Entity::write and Entity::read stream ten fields in a fixed order, and each
   entity wraps them in its own tag and maps.  Every entity is written alone
   and read into a fresh mesh, and compared field by field through
   Entity::operator==, whose subsets compare through Subset::operator==. */
TEST_CASE("entity: write and read round trip every field", "[mesh][io]") {
  Bare_Mesh src;
  populate(src.mesh, true);
  Bare_Mesh dst;
  auto const from = entities_of(src.mesh);
  auto const to = entities_of(dst.mesh);
  for (std::size_t i = 0; i < from.size(); ++i) {
    auto const &[name, original] = from[i];
    Ume::SOA_Idx::Entity &copy = *to[i].second;
    INFO("entity " << name);
    std::stringstream stream;
    original->write(stream);
    copy.read(stream);
    CHECK(exhausted(stream));
    CHECK(copy.local_size() == original->local_size());
    CHECK(copy.size() == original->size());
    CHECK(copy == *original);
  }

  /* Positive control: a difference in any one of the fields that the
     comparison above relies on, the subset fields among them, makes the
     entities unequal. */
  using Change = void (*)(Ume::SOA_Idx::Entity &);
  std::pair<char const *, Change> const changes[] = {
      {"lsize",
          [](Ume::SOA_Idx::Entity &e) {
            e.resize(e.local_size() - 1, e.size(),
                static_cast<int>(e.cpy_idx.size()));
          }},
      {"mask", [](Ume::SOA_Idx::Entity &e) { ++e.mask.back(); }},
      {"comm_type", [](Ume::SOA_Idx::Entity &e) { ++e.comm_type.back(); }},
      {"cpy_idx", [](Ume::SOA_Idx::Entity &e) { ++e.cpy_idx.back(); }},
      {"src_pe", [](Ume::SOA_Idx::Entity &e) { ++e.src_pe.back(); }},
      {"src_idx", [](Ume::SOA_Idx::Entity &e) { ++e.src_idx.back(); }},
      {"ghost_mask", [](Ume::SOA_Idx::Entity &e) { ++e.ghost_mask.back(); }},
      {"myCpys", [](Ume::SOA_Idx::Entity &e) { ++e.myCpys.back().pe; }},
      {"mySrcs",
          [](Ume::SOA_Idx::Entity &e) { ++e.mySrcs.back().elements.back(); }},
      {"subset name",
          [](Ume::SOA_Idx::Entity &e) { e.subsets.back().name += '_'; }},
      {"subset lsize",
          [](Ume::SOA_Idx::Entity &e) { ++e.subsets.back().lsize; }},
      {"subset elements",
          [](Ume::SOA_Idx::Entity &e) { ++e.subsets.back().elements.back(); }},
      {"subset mask",
          [](Ume::SOA_Idx::Entity &e) { ++e.subsets.back().mask.back(); }},
  };
  std::stringstream zones;
  src.mesh.zones.write(zones);
  for (auto const &[field, change] : changes) {
    INFO("field " << field);
    Bare_Mesh changed;
    std::istringstream stream{zones.str()};
    changed.mesh.zones.read(stream);
    REQUIRE(changed.mesh.zones == src.mesh.zones);
    change(changed.mesh.zones);
    CHECK_FALSE(changed.mesh.zones == src.mesh.zones);
  }
}

/* Mesh::write and Mesh::read are the binary mesh format: the mesh scalars,
   then each entity in a fixed order, the iotas only when `dump_iotas` says
   so.  A mesh read back from what was written compares equal to the
   original, entity by entity. */
TEST_CASE("mesh: write and read round trip", "[mesh][io]") {
  bool const with_iotas = GENERATE(true, false);
  INFO("with iotas " << with_iotas);
  Bare_Mesh src;
  populate(src.mesh, with_iotas);
  std::stringstream stream;
  src.mesh.write(stream);

  Bare_Mesh dst;
  dst.mesh.version_header = false;
  dst.mesh.dump_iotas = !with_iotas;
  dst.mesh.read(stream);
  CHECK(exhausted(stream));
  CHECK(dst.mesh.version_header);
  CHECK(dst.mesh.iotas.size() == src.mesh.iotas.size());
  CHECK(dst.mesh == src.mesh);

  /* Positive control: one face map entry, or one mesh scalar, differing makes
     the meshes unequal. */
  auto &face_to_zone = dst.mesh.ds->access_intv("m:f>z1");
  ++face_to_zone.front();
  CHECK_FALSE(dst.mesh == src.mesh);
  --face_to_zone.front();
  REQUIRE(dst.mesh == src.mesh);
  ++dst.mesh.numpe;
  CHECK_FALSE(dst.mesh == src.mesh);
}

/* A copy has one source, so an entity's myCpys names each copy once across
   all of its remotes, and a scatter's OVERWRITE unpack writes each copy once.
   That used to be asserted inside every OVERWRITE unpack, with a std::set
   built on every call in every build; Mesh::read checks it once instead, and
   fails the stream on a mesh that breaks it.  A source can have a copy on
   several remotes, so mySrcs may name an entity twice, and has to be let
   through. */
TEST_CASE("mesh: read refuses a copy listed twice", "[mesh][io][comm]") {
  using Ume::SOA_Idx::Entity;
  using Change = void (*)(Entity &);
  struct Case {
    char const *what;
    Change change;
    bool refused;
  };
  Case const cases[] = {
      // Positive control: the same mesh without a repeated copy reads.
      {"nothing", [](Entity &) {}, false},
      {"a source under two remotes",
          [](Entity &e) {
            e.mySrcs.push_back(
                {e.mySrcs.front().pe + 1, {e.mySrcs.front().elements.front()}});
          },
          false},
      {"a copy under two remotes",
          [](Entity &e) {
            e.myCpys.back().elements.push_back(
                e.myCpys.front().elements.front());
          },
          true},
      {"a copy twice under one remote",
          [](Entity &e) {
            e.myCpys.front().elements.push_back(
                e.myCpys.front().elements.front());
          },
          true},
  };

  for (std::size_t i = 0; i < 7; ++i) {
    for (auto const &[what, change, refused] : cases) {
      Bare_Mesh src;
      populate(src.mesh, true);
      /* Named, so the binding below does not refer into a temporary array
         destroyed at the end of its statement. */
      auto const entities = entities_of(src.mesh);
      auto const &[name, entity] = entities[i];
      INFO("entity " << name << ", changed " << what);
      change(*entity);
      std::stringstream stream;
      src.mesh.write(stream);

      Bare_Mesh dst;
      dst.mesh.read(stream);
      CHECK(stream.fail() == refused);
      if (!refused) {
        CHECK(dst.mesh == src.mesh);
        /* Positive control: the comparison sees the change that was read
           back, so the mesh read differs from an unchanged one exactly when
           something was changed. */
        Bare_Mesh unchanged;
        populate(unchanged.mesh, true);
        CHECK((dst.mesh == unchanged.mesh) == (change == cases[0].change));
      }
    }
  }
}

namespace {

/* `tag` as a mesh file lays it out: its length, then its characters. */
std::string encoded_tag(std::string const &tag) {
  std::ostringstream os;
  Ume::write_bin(os, tag);
  return os.str();
}

} // namespace

/* Each entity in a mesh file opens with a tag, and so does each of its two
   neighbor lists.  The tags were checked with asserts, which ran on a stream
   that had already failed as well: a Debug build reading a file cut short died
   on SIGABRT before its caller could see the failed stream and report it.  A
   wrong tag now fails the stream and Mesh::read stops there: the entities
   before it are read, and the one it opens and every later one are left as
   they were.  Each tag in turn has its last character changed, so that the
   bytes after it stay where they were and only the tag can fail the read. */
TEST_CASE("mesh: read fails the stream on a wrong tag", "[mesh][io]") {
  Bare_Mesh src;
  populate(src.mesh, true);
  std::ostringstream written;
  src.mesh.write(written);
  std::string const bytes = written.str();
  auto const original = entities_of(src.mesh);

  struct Tag {
    std::string what;
    std::size_t last; // offset of its last character in `bytes`
    std::size_t index; // of the entity it belongs to, in `original`
    bool opens_entity; // rather than one of the entity's neighbor lists
  };
  std::vector<Tag> tags;
  std::size_t from = 0;
  auto const find_tag = [&bytes, &from](std::string const &tag) {
    std::size_t const at = bytes.find(tag, from);
    REQUIRE(at != std::string::npos);
    from = at + tag.size();
    return from - 1;
  };
  std::string const neighbors = encoded_tag("neighbors");
  for (std::size_t i = 0; i < original.size(); ++i) {
    std::string const name = original[i].first;
    tags.push_back({name, find_tag(encoded_tag(name)), i, true});
    for (char const *const list : {" myCpys", " mySrcs"})
      tags.push_back({name + list, find_tag(neighbors), i, false});
  }

  /* Positive controls: the bytes as written read back whole, and each entity
     of a mesh that has not been read differs from the one written.  So an
     entity found below to be as it was would have been read had the read gone
     on. */
  {
    std::istringstream stream{bytes};
    Bare_Mesh dst;
    dst.mesh.read(stream);
    REQUIRE_FALSE(stream.fail());
    REQUIRE(dst.mesh == src.mesh);
  }
  Bare_Mesh fresh;
  auto const unread = entities_of(fresh.mesh);
  for (std::size_t j = 0; j < unread.size(); ++j)
    REQUIRE_FALSE(*unread[j].second == *original[j].second);

  for (auto const &[what, last, index, opens_entity] : tags) {
    INFO("wrong tag: " << what);
    std::string changed = bytes;
    changed[last] = '?';
    std::istringstream stream{changed};
    Bare_Mesh dst;
    dst.mesh.read(stream);
    CHECK(stream.fail());
    /* A wrong neighbor list tag leaves its entity part read, which is not
       checked. */
    auto const read = entities_of(dst.mesh);
    for (std::size_t j = 0; j < read.size(); ++j) {
      INFO("entity " << read[j].first);
      if (j < index)
        CHECK(*read[j].second == *original[j].second);
      else if (j > index || opens_entity)
        CHECK(*read[j].second == *unread[j].second);
    }
  }
}

/* A change to the order of the format made in write and read alike passes
   both round trips above, and misreads every existing input file.  So the
   layout itself is pinned: the zones (a tag and the Entity fields, nothing
   else) and the mesh (its scalars, then its entities), each against a stream
   built from the format's order. */
TEST_CASE("mesh: write lays fields out in the file order", "[mesh][io]") {
  Bare_Mesh src;
  populate(src.mesh, true);
  Mesh const &mesh = src.mesh;
  auto const &zones = mesh.zones;

  /* `swap` exchanges src_pe and src_idx, for the positive control. */
  auto const zones_layout = [&zones](bool const swap) {
    std::ostringstream os;
    Ume::write_bin(os, std::string{"zones"});
    Ume::write_bin(os, zones.local_size());
    Ume::write_bin(os, zones.mask);
    Ume::write_bin(os, zones.comm_type);
    Ume::write_bin(os, zones.cpy_idx);
    Ume::write_bin(os, swap ? zones.src_idx : zones.src_pe);
    Ume::write_bin(os, swap ? zones.src_pe : zones.src_idx);
    Ume::write_bin(os, zones.ghost_mask);
    Ume::write_bin<Ume::Comm::Neighbors>(os, zones.myCpys);
    Ume::write_bin<Ume::Comm::Neighbors>(os, zones.mySrcs);
    Ume::write_bin(os, zones.subsets.size());
    for (auto const &subset : zones.subsets) {
      Ume::write_bin(os, subset.name);
      Ume::write_bin(os, subset.lsize);
      Ume::write_bin(os, subset.elements);
      Ume::write_bin(os, subset.mask);
      os << '\n';
    }
    os << "\n\n";
    return os.str();
  };
  std::ostringstream written;
  zones.write(written);
  CHECK(written.str() == zones_layout(false));
  CHECK_FALSE(written.str() == zones_layout(true));

  /* `swap` exchanges edges and faces, for the positive control. */
  auto const mesh_layout = [&mesh](bool const swap) {
    std::ostringstream os;
    Ume::write_bin(os, mesh.ivtag);
    Ume::write_bin(os, mesh.mype);
    Ume::write_bin(os, mesh.numpe);
    Ume::write_bin(os, mesh.geo);
    Ume::write_bin(os, mesh.dump_iotas);
    Ume::SOA_Idx::Entity const *const entities[] = {&mesh.points,
        swap ? static_cast<Ume::SOA_Idx::Entity const *>(&mesh.faces)
             : &mesh.edges,
        swap ? static_cast<Ume::SOA_Idx::Entity const *>(&mesh.edges)
             : &mesh.faces,
        &mesh.sides, &mesh.corners, &mesh.zones, &mesh.iotas};
    for (Ume::SOA_Idx::Entity const *const e : entities)
      e->write(os);
    return os.str();
  };
  written.str("");
  mesh.write(written);
  CHECK(written.str() == mesh_layout(false));
  CHECK_FALSE(written.str() == mesh_layout(true));
}

/* Mesh's header scalars had no initializers, so a mesh read from a stream
   that had failed, or built in place and written without setting them,
   carried whatever the storage held: a read of a 0-3 byte file fails before
   the header and left dump_iotas indeterminate, and Mesh::read branches on
   it.  The storage is filled with a pattern first so
   that a constructor that leaves any of them alone is seen to.  ivtag
   defaults to the version Mesh::write lays out, so a default mesh written
   reads back as that; the rest describe rank 0 of 1, Cartesian, with no
   iotas.  A bool or an enum holding the fill is not a valid value of its
   type, so those are compared by their bytes. */
TEST_CASE("mesh: default construction sets every header scalar", "[mesh][io]") {
  constexpr std::byte fill{0xA5};
  // Positive controls: the fill, left in place, is none of the values
  // expected.  false and CARTESIAN are all zero bytes.
  static_assert(static_cast<int>(0xA5A5A5A5U) != UME_VERSION_2);
  static_assert(static_cast<int>(0xA5A5A5A5U) != 0);
  static_assert(static_cast<int>(0xA5A5A5A5U) != 1);
  static_assert(fill != std::byte{0} && Mesh::CARTESIAN == 0);

  alignas(Mesh) std::array<std::byte, sizeof(Mesh)> storage;
  storage.fill(fill);
  Mesh *const mesh = ::new (storage.data()) Mesh;
  CHECK(mesh->ivtag == UME_VERSION_2);
  CHECK(bytes_of(mesh->version_header) == bytes_of(false));
  CHECK(mesh->mype == 0);
  CHECK(mesh->numpe == 1);
  CHECK(bytes_of(mesh->geo) == bytes_of(Mesh::CARTESIAN));
  CHECK(bytes_of(mesh->dump_iotas) == bytes_of(false));
  std::destroy_at(mesh);
}
