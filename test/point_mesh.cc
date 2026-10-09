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
  \file point_mesh.cc

  Writes, or prints the point coordinates of, a single-rank Ume binary mesh:

      point_mesh write <output.ume> [<x>[,<y>,<z>]...]
      point_mesh print <input.ume>

  `write` makes a mesh whose only entities are local points, one per argument,
  at (x, y, z), or at (x, 0, 0) where only <x> is given.  `print` writes one
  line per point of any mesh, "point <i>: x y z", with each coordinate in its
  shortest round-trip form.  The scale_mesh tests write a mesh with the first,
  run scale_mesh on it, and read where the stitched copy of each point landed
  with the second.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include <array>
#include <charconv>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

/* The number in `text`, or nothing if it is not entirely a `double`. */
std::optional<double> parse_coord(std::string_view const text) {
  double value{};
  auto const [end, ec] =
      std::from_chars(text.data(), text.data() + text.size(), value);
  if (ec != std::errc{} || end != text.data() + text.size())
    return std::nullopt;
  return value;
}

/* The point in `text`, "x" or "x,y,z", or nothing if it is neither. */
std::optional<std::array<double, 3>> parse_point(std::string_view const text) {
  std::array<double, 3> point{};
  std::size_t n{0};
  for (auto const part : std::views::split(text, ',')) {
    auto const coord = parse_coord(std::string_view{part.begin(), part.end()});
    if (n == point.size() || !coord)
      return std::nullopt;
    point[n++] = *coord;
  }
  if (n != 1 && n != point.size())
    return std::nullopt;
  return point;
}

int write_mesh(char const *const fname, std::span<char *const> const args) {
  std::vector<std::array<double, 3>> coords;
  for (std::string_view const text : args) {
    auto const point = parse_point(text);
    if (!point) {
      std::cerr << "point_mesh: \"" << text << "\" is not a point\n";
      return EXIT_FAILURE;
    }
    coords.push_back(*point);
  }

  Ume::SOA_Idx::Mesh mesh;
  mesh.ivtag = UME_VERSION_2;
  mesh.mype = 0;
  mesh.numpe = 1;
  mesh.geo = Ume::SOA_Idx::Mesh::CARTESIAN;
  mesh.dump_iotas = false;
  Ume::SOA_Idx::Entity *const empty[] = {&mesh.zones, &mesh.edges, &mesh.faces,
      &mesh.sides, &mesh.corners, &mesh.iotas};
  for (Ume::SOA_Idx::Entity *const e : empty)
    e->resize(0, 0, 0);
  int const npoints = static_cast<int>(coords.size());
  mesh.points.resize(npoints, npoints, 0);
  auto &pcoord = mesh.ds->access_vec3v("pcoord");
  for (int p = 0; p < npoints; ++p)
    pcoord[p] = Ume::SOA_Idx::PtCoord{std::array<double, 3>{coords[p]}};

  std::ofstream os(fname, std::ios::binary);
  if (!os) {
    std::cerr << "point_mesh: cannot open \"" << fname << "\" for writing\n";
    return EXIT_FAILURE;
  }
  mesh.write(os);
  os.close();
  if (!os) {
    std::cerr << "point_mesh: failed writing \"" << fname << "\"\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int print_mesh(char const *const fname) {
  std::ifstream is(fname, std::ios::binary);
  if (!is) {
    std::cerr << "point_mesh: cannot open \"" << fname << "\" for reading\n";
    return EXIT_FAILURE;
  }
  Ume::SOA_Idx::Mesh mesh;
  mesh.read(is);
  if (!is) {
    std::cerr << "point_mesh: failed reading \"" << fname << "\"\n";
    return EXIT_FAILURE;
  }
  auto const &pcoord = mesh.ds->caccess_vec3v("pcoord");
  for (int p = 0; p < mesh.points.size(); ++p)
    std::cout << std::format(
        "point {}: {} {} {}\n", p, pcoord[p][0], pcoord[p][1], pcoord[p][2]);
  return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[]) {
  std::span<char *const> const args(argv, static_cast<std::size_t>(argc));
  if (args.size() >= 3 && std::string_view{args[1]} == "write")
    return write_mesh(args[2], args.subspan(3));
  if (args.size() == 3 && std::string_view{args[1]} == "print")
    return print_mesh(args[2]);
  std::cerr << "Usage: point_mesh write <output.ume> [<x>[,<y>,<z>]...]\n"
               "       point_mesh print <input.ume>\n";
  return EXIT_FAILURE;
}
