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
  \file entity_mesh.cc

  Writes, or prints the entity data of, a small Ume binary mesh:

      entity_mesh write <output.ume> [--numpe=<n>] [--ghost-zone]
      entity_mesh print <input.ume>

  `write` makes a single-rank mesh with every entity, iotas included, whose
  masks, communication types, subsets and index maps all hold values a copy
  can be told apart from a zero fill by, and with the sentinel -1 in `m:f>z2`
  (a boundary face) and `m:s>s5` (a side with no neighbor).  The entity counts
  differ, so that an index map offset by the wrong entity's count shows.  The
  connectivity is consistent only as far as every index is in range: no kernel
  runs on it.  `--numpe` records that many ranks in the header, and
  `--ghost-zone` adds one ghost zone; the scale_mesh tests use those to see a
  mesh refused.

  `print` writes the entity data of any mesh, one line per array: each
  entity's sizes, `mask`, `comm_type` and subsets, then each index map.  The
  scale_mesh tests write a mesh with the first, scale it, and match what the
  stitched copy of each entity holds with the second.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include <array>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using Ume::SOA_Idx::Entity;
using Ume::SOA_Idx::Mesh;

/* The count in `text`, or nothing if it is not entirely a positive `int`. */
std::optional<int> parse_count(std::string_view const text) {
  int value{};
  auto const [end, ec] =
      std::from_chars(text.data(), text.data() + text.size(), value);
  if (ec != std::errc{} || end != text.data() + text.size() || value < 1)
    return std::nullopt;
  return value;
}

/* Each entity in the order Mesh::write lays them out, with its name. */
auto entities(Mesh &mesh) {
  return std::array<std::pair<char const *, Entity *>, 7>{{
      {"points", &mesh.points},
      {"edges", &mesh.edges},
      {"faces", &mesh.faces},
      {"sides", &mesh.sides},
      {"corners", &mesh.corners},
      {"zones", &mesh.zones},
      {"iotas", &mesh.iotas},
  }};
}

/* Every index map, in the order the entities' write() lays them out. */
constexpr std::array<char const *, 22> index_maps{"m:e>p1", "m:e>p2", "m:f>z1",
    "m:f>z2", "m:s>z", "m:s>p1", "m:s>p2", "m:s>e", "m:s>f", "m:s>c1", "m:s>c2",
    "m:s>s2", "m:s>s3", "m:s>s4", "m:s>s5", "m:c>p", "m:c>z", "m:a>z", "m:a>f",
    "m:a>p", "m:a>e", "m:a>s"};

int write_mesh(
    char const *const fname, int const numpe, bool const ghost_zone) {
  Mesh mesh;
  mesh.ivtag = UME_VERSION_2;
  mesh.mype = 0;
  mesh.numpe = numpe;
  mesh.geo = Mesh::CARTESIAN;
  mesh.dump_iotas = true;

  mesh.points.resize(2, 2, 0);
  mesh.edges.resize(1, 1, 0);
  mesh.faces.resize(3, 3, 0);
  mesh.sides.resize(2, 2, 0);
  mesh.corners.resize(2, 2, 0);
  if (ghost_zone)
    mesh.zones.resize(2, 3, 1);
  else
    mesh.zones.resize(2, 2, 0);
  mesh.iotas.resize(1, 1, 0);

  for (auto const &[name, entity] : entities(mesh)) {
    for (int i = 0; i < entity->size(); ++i) {
      entity->mask[i] = (i % 2 == 0) ? 1 : -1;
      entity->comm_type[i] = Entity::INTERNAL + i;
    }
  }
  if (ghost_zone) {
    mesh.zones.comm_type[2] = Entity::GHOST;
    mesh.zones.cpy_idx = {2};
    mesh.zones.src_pe = {0};
    mesh.zones.src_idx = {1};
    mesh.zones.ghost_mask = {1};
  }
  mesh.points.subsets = {{"corner", 1, {1}, {3}}};
  mesh.faces.subsets = {{"boundary", 2, {1, 2}, {-1, -2}}};
  mesh.zones.subsets = {{"inner", 1, {0}, {1}}, {"all", 2, {0, 1}, {1, -1}}};

  auto &pcoord = mesh.ds->access_vec3v("pcoord");
  pcoord[0] = Ume::SOA_Idx::PtCoord{std::array{0.0, 0.0, 0.0}};
  pcoord[1] = Ume::SOA_Idx::PtCoord{std::array{1.0, 2.0, 3.0}};

  std::pair<char const *, std::vector<int>> const maps[] = {
      {"m:e>p1", {0}},
      {"m:e>p2", {1}},
      {"m:f>z1", {0, 0, 1}},
      {"m:f>z2", {1, -1, -1}},
      {"m:s>z", {0, 1}},
      {"m:s>p1", {0, 1}},
      {"m:s>p2", {1, 0}},
      {"m:s>e", {0, 0}},
      {"m:s>f", {0, 2}},
      {"m:s>c1", {0, 1}},
      {"m:s>c2", {1, 0}},
      {"m:s>s2", {1, 0}},
      {"m:s>s3", {0, 1}},
      {"m:s>s4", {1, 0}},
      {"m:s>s5", {1, -1}},
      {"m:c>p", {0, 1}},
      {"m:c>z", {0, 1}},
      {"m:a>z", {1}},
      {"m:a>f", {2}},
      {"m:a>p", {1}},
      {"m:a>e", {0}},
      {"m:a>s", {1}},
  };
  for (auto const &[name, values] : maps)
    mesh.ds->access_intv(name) = values;

  std::ofstream os(fname, std::ios::binary);
  if (!os) {
    std::cerr << "entity_mesh: cannot open \"" << fname << "\" for writing\n";
    return EXIT_FAILURE;
  }
  mesh.write(os);
  os.close();
  if (!os) {
    std::cerr << "entity_mesh: failed writing \"" << fname << "\"\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

template <typename T> void print_values(std::vector<T> const &values) {
  for (T const v : values)
    std::cout << ' ' << v;
}

int print_mesh(char const *const fname) {
  std::ifstream is(fname, std::ios::binary);
  if (!is) {
    std::cerr << "entity_mesh: cannot open \"" << fname << "\" for reading\n";
    return EXIT_FAILURE;
  }
  Mesh mesh;
  mesh.read(is);
  if (!is) {
    std::cerr << "entity_mesh: failed reading \"" << fname << "\"\n";
    return EXIT_FAILURE;
  }
  for (auto const &[name, entity] : entities(mesh)) {
    std::cout << name << " lsize " << entity->local_size() << " size "
              << entity->size() << '\n';
    std::cout << name << " mask:";
    print_values(entity->mask);
    std::cout << '\n' << name << " comm_type:";
    print_values(entity->comm_type);
    std::cout << '\n';
    for (auto const &subset : entity->subsets) {
      std::cout << name << " subset " << subset.name << " lsize "
                << subset.lsize << " elements:";
      print_values(subset.elements);
      std::cout << " mask:";
      print_values(subset.mask);
      std::cout << '\n';
    }
  }
  for (char const *const name : index_maps) {
    std::cout << name << ':';
    print_values(mesh.ds->caccess_intv(name));
    std::cout << '\n';
  }
  return EXIT_SUCCESS;
}

int usage() {
  std::cerr << "Usage: entity_mesh write <output.ume> [--numpe=<n>] "
               "[--ghost-zone]\n"
               "       entity_mesh print <input.ume>\n";
  return EXIT_FAILURE;
}

} // namespace

int main(int argc, char *argv[]) {
  std::span<char *const> const args(argv, static_cast<std::size_t>(argc));
  if (args.size() == 3 && std::string_view{args[1]} == "print")
    return print_mesh(args[2]);
  if (args.size() < 3 || std::string_view{args[1]} != "write")
    return usage();

  int numpe{1};
  bool ghost_zone{false};
  constexpr std::string_view numpe_flag{"--numpe="};
  for (std::string_view const arg : args.subspan(3)) {
    if (arg == "--ghost-zone") {
      ghost_zone = true;
    } else if (arg.starts_with(numpe_flag)) {
      auto const count = parse_count(arg.substr(numpe_flag.size()));
      if (!count) {
        std::cerr << "entity_mesh: \"" << arg << "\" is not a rank count\n";
        return EXIT_FAILURE;
      }
      numpe = *count;
    } else {
      return usage();
    }
  }
  return write_mesh(args[2], numpe, ghost_zone);
}
