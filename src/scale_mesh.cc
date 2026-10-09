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
  \file scale_mesh.cc

  Reads a whole (single-rank, ghost-free) Ume binary mesh, stitches copies of
  it together to make a mesh `scale` times larger, and writes the result.

  The input is '<basename>.00000.ume'; a partitioned mesh, or one with ghosts,
  is refused.  The output is '<basename>.<scale>.00000.ume', both numbers
  zero-filled to five digits.  `scale` has to be a power of 2.
*/

#include "Ume/Comm_MPI.hh"
#include "Ume/DS_Types.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh);

bool is_serial(Ume::SOA_Idx::Mesh const &mesh);

void scale_mesh(int const scale, Ume::SOA_Idx::Mesh &mesh);

bool write_mesh(char const *const basename, int const mype, int const scale,
    Ume::SOA_Idx::Mesh &mesh);

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "Usage: ./scale_mesh <input-basename> <scale-factor>"
              << std::endl;
    std::cerr << "<input-basename> is the UME Input File Basename (exclude "
                 "rank number and file extension"
              << std::endl;
    std::cerr << "<scale-factor> is the amount to scale the original mesh by. "
                 "Only powers of 2 are valid."
              << std::endl;
    return 1;
  }

  /* The whole argument has to be the number, and it has to be positive before
     `scale & (scale - 1)` means anything: on the most negative int the
     subtraction overflows.  Checked before the mesh is read, as ume_mpi checks
     its arguments, so that a mistyped scale fails before a read that can take
     minutes rather than after it; and before MPI starts, so that this exit,
     like the usage one above, has no MPI to finalize. */
  std::string_view const scale_arg{argv[2]};
  int scale = 0;
  auto const [end, ec] = std::from_chars(
      scale_arg.data(), scale_arg.data() + scale_arg.size(), scale);
  if (ec != std::errc{} || end != scale_arg.data() + scale_arg.size() ||
      scale < 1 || (scale & (scale - 1)) != 0) {
    std::cerr << "Scale must be a power of 2, got \"" << scale_arg << '"'
              << std::endl;
    return 1;
  }

  Ume::SOA_Idx::Mesh mesh;

  int mype = 0;
#ifdef HAVE_MPI
  Ume::Comm::MPI comm(&argc, &argv);
  mesh.comm = &comm;

  mype = comm.pe();
#endif
  if (mype == 0)
    std::cout << "Initializing mesh..." << std::endl;

  /* From here on every exit goes through the one comm.stop() below: a failure
     that returned from main left MPI unfinalized, which a launcher reports as
     an improper exit. */
  int status = 0;
  if (!read_mesh(argv[1], mype, mesh) || !is_serial(mesh)) {
    std::cerr << "Aborting." << std::endl;
    status = 1;
  } else {
    if (mype == 0)
      std::cout << "Scaling mesh by a factor of " << scale << "..."
                << std::endl;

    scale_mesh(scale, mesh);

    if (mype == 0)
      std::cout << "Writing scaled mesh..." << std::endl;

    if (!write_mesh(argv[1], mype, scale, mesh)) {
      std::cerr << "Aborting." << std::endl;
      status = 1;
    } else if (mype == 0) {
      std::cout << "Done." << std::endl;
    }
  }
#ifdef HAVE_MPI
  comm.stop();
#endif
  return status;
}

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh) {
  std::string const fname = Ume::mesh_filename(basename, mype);
  std::ifstream is(fname);
  if (!is) {
    std::cerr << "Unable to open file \"" << fname << "\" for reading."
              << std::endl;
    return false;
  }
  mesh.read(is);
  is.close();
  if (!is) {
    std::cerr << "Unable to read a mesh from file \"" << fname << "\"."
              << std::endl;
    return false;
  }
  return true;
}

/*! Whether `mesh` is one whole mesh: one rank, and no ghosts on any entity.
    stitch() copies each entity's elements in a block after the originals, and
    has no exchange lists to give the copies; a ghost's copy would also land
    among the local elements, where nothing expects a ghost.  Says why not on
    std::cerr when it is not. */
bool is_serial(Ume::SOA_Idx::Mesh const &mesh) {
  if (mesh.numpe != 1) {
    std::cerr << "scale_mesh needs a single-rank mesh; this is rank "
              << mesh.mype << " of " << mesh.numpe << '.' << std::endl;
    return false;
  }
  std::pair<char const *, Ume::SOA_Idx::Entity const *> const entities[] = {
      {"points", &mesh.points}, {"edges", &mesh.edges}, {"faces", &mesh.faces},
      {"sides", &mesh.sides}, {"corners", &mesh.corners},
      {"zones", &mesh.zones}, {"iotas", &mesh.iotas}};
  for (auto const &[name, entity] : entities) {
    if (entity->ghost_local_size() != 0) {
      std::cerr << "scale_mesh needs a mesh without ghosts; its " << name
                << " [" << entity->local_size() << ", " << entity->size()
                << ") are ghosts." << std::endl;
      return false;
    }
  }
  return true;
}

//! The extent, max - min, of the point coordinates along `dim`; 0 if none.
double get_extent(Ume::SOA_Idx::Mesh const &mesh, int const dim) {
  auto const &pcoord = mesh.ds->caccess_vec3v("pcoord");
  if (pcoord.empty())
    return 0.0;
  auto const [lo, hi] = std::ranges::minmax(pcoord |
      std::views::transform(
          [dim](Ume::DS_Types::VEC3_T const &p) { return p[dim]; }));
  return hi - lo;
}

//! `index` moved by `delta`, or `index` itself if it is a -1 sentinel.
constexpr int offset_index(int const index, int const delta) {
  return index < 0 ? index : index + delta;
}

/*! Double `entity`, with the second half a copy of the first: its mask and
    communication type, and each subset extended by the copies of its
    elements.  The entity has no ghosts (is_serial), so every subset element
    is local and the copies stay in the subset's local range.  The index maps
    and coordinates the entity's resize() grows are left to the caller. */
void double_entity_count(Ume::SOA_Idx::Entity &entity) {
  assert(entity.ghost_local_size() == 0);
  int const original_total = entity.size();
  entity.resize(original_total * 2, original_total * 2, 0);
  std::ranges::copy_n(entity.mask.begin(), original_total,
      entity.mask.begin() + original_total);
  std::ranges::copy_n(entity.comm_type.begin(), original_total,
      entity.comm_type.begin() + original_total);

  for (auto &subset : entity.subsets) {
    std::vector<int> elements;
    elements.reserve(subset.elements.size());
    std::ranges::transform(subset.elements, std::back_inserter(elements),
        [original_total](
            int const e) { return offset_index(e, original_total); });
    subset.elements.insert(
        subset.elements.end(), elements.begin(), elements.end());
    std::vector<short> const mask{subset.mask};
    subset.mask.insert(subset.mask.end(), mask.begin(), mask.end());
    subset.lsize *= 2;
  }
}

void update_coords(Ume::DS_Types::VEC3V_T &coords, const int iter_start,
    const int iter_end, const double delta, const int dim) {
  for (int c = iter_start; c < iter_end; ++c) {
    coords[c] = coords[c - iter_start];
    coords[c][dim] += delta;
  }
}

void update_entity(Ume::DS_Types::INTV_T &map, const int iter_start,
    const int iter_end, const int delta) {
  for (int e = iter_start; e < iter_end; ++e) {
    map[e] = offset_index(map[e - iter_start], delta);
  }
}

void stitch(Ume::SOA_Idx::Mesh &mesh, const int dim) {
  //! ----- Points -----
  // Double Number of Points
  // The copy sits against the original's +dim face, whatever its position.
  double const extent = get_extent(mesh, dim);

  int original_points_total = mesh.points.size();
  int new_points_total = original_points_total * 2;
  double_entity_count(mesh.points);

  // Update Point Coordinates of new Points
  auto &new_pcoords = mesh.ds->access_vec3v("pcoord");
  update_coords(
      new_pcoords, original_points_total, new_points_total, extent, dim);

  //! ----- Zones -----
  // Double Number of Zones
  int original_zones_total = mesh.zones.size();
  // int new_zones_total = original_zones_total * 2;
  double_entity_count(mesh.zones);

  //! ----- Edges -----
  // Double Number of Edges
  int original_edges_total = mesh.edges.size();
  int new_edges_total = original_edges_total * 2;
  double_entity_count(mesh.edges);

  // Update Edge Maps of new Edges
  auto &e_to_p1_map = mesh.ds->access_intv("m:e>p1");
  auto &e_to_p2_map = mesh.ds->access_intv("m:e>p2");

  update_entity(e_to_p1_map, original_edges_total, new_edges_total,
      original_points_total);
  update_entity(e_to_p2_map, original_edges_total, new_edges_total,
      original_points_total);

  //! ----- Faces -----
  // Double Number of Faces
  int original_faces_total = mesh.faces.size();
  int new_faces_total = original_faces_total * 2;
  double_entity_count(mesh.faces);

  // Update Face Maps of new Faces
  auto &f_to_z1_map = mesh.ds->access_intv("m:f>z1");
  auto &f_to_z2_map = mesh.ds->access_intv("m:f>z2");

  update_entity(
      f_to_z1_map, original_faces_total, new_faces_total, original_zones_total);
  update_entity(
      f_to_z2_map, original_faces_total, new_faces_total, original_zones_total);

  //! ----- Corners -----
  // Double Number of Corners
  int original_corners_total = mesh.corners.size();
  int new_corners_total = original_corners_total * 2;
  double_entity_count(mesh.corners);

  // Update Corner Maps of new Corners
  auto &c_to_p_map = mesh.ds->access_intv("m:c>p");
  auto &c_to_z_map = mesh.ds->access_intv("m:c>z");

  update_entity(c_to_p_map, original_corners_total, new_corners_total,
      original_points_total);
  update_entity(c_to_z_map, original_corners_total, new_corners_total,
      original_zones_total);

  //! ----- Sides -----
  // Double Number of Sides
  int original_sides_total = mesh.sides.size();
  int new_sides_total = original_sides_total * 2;
  double_entity_count(mesh.sides);

  // Update Side Maps of new Sides
  auto &s_to_z_map = mesh.ds->access_intv("m:s>z");
  auto &s_to_e_map = mesh.ds->access_intv("m:s>e");
  auto &s_to_p1_map = mesh.ds->access_intv("m:s>p1");
  auto &s_to_p2_map = mesh.ds->access_intv("m:s>p2");
  auto &s_to_f_map = mesh.ds->access_intv("m:s>f");
  auto &s_to_c1_map = mesh.ds->access_intv("m:s>c1");
  auto &s_to_c2_map = mesh.ds->access_intv("m:s>c2");
  auto &s_to_s2_map = mesh.ds->access_intv("m:s>s2");
  auto &s_to_s3_map = mesh.ds->access_intv("m:s>s3");
  auto &s_to_s4_map = mesh.ds->access_intv("m:s>s4");
  auto &s_to_s5_map = mesh.ds->access_intv("m:s>s5");

  update_entity(
      s_to_z_map, original_sides_total, new_sides_total, original_zones_total);
  update_entity(
      s_to_e_map, original_sides_total, new_sides_total, original_edges_total);
  update_entity(s_to_p1_map, original_sides_total, new_sides_total,
      original_points_total);
  update_entity(s_to_p2_map, original_sides_total, new_sides_total,
      original_points_total);
  update_entity(
      s_to_f_map, original_sides_total, new_sides_total, original_faces_total);
  update_entity(s_to_c1_map, original_sides_total, new_sides_total,
      original_corners_total);
  update_entity(s_to_c2_map, original_sides_total, new_sides_total,
      original_corners_total);
  update_entity(
      s_to_s2_map, original_sides_total, new_sides_total, original_sides_total);
  update_entity(
      s_to_s3_map, original_sides_total, new_sides_total, original_sides_total);
  update_entity(
      s_to_s4_map, original_sides_total, new_sides_total, original_sides_total);
  update_entity(
      s_to_s5_map, original_sides_total, new_sides_total, original_sides_total);

  //! ----- Iotas -----
  // Double Number of Iotas
  int original_iotas_total = mesh.iotas.size();
  int new_iotas_total = original_iotas_total * 2;
  double_entity_count(mesh.iotas);

  // Update Corner Maps of new Corners
  if (mesh.dump_iotas) {
    auto &a_to_z_map = mesh.ds->access_intv("m:a>z");
    auto &a_to_f_map = mesh.ds->access_intv("m:a>f");
    auto &a_to_p_map = mesh.ds->access_intv("m:a>p");
    auto &a_to_e_map = mesh.ds->access_intv("m:a>e");
    auto &a_to_s_map = mesh.ds->access_intv("m:a>s");

    update_entity(a_to_z_map, original_iotas_total, new_iotas_total,
        original_zones_total);
    update_entity(a_to_f_map, original_iotas_total, new_iotas_total,
        original_faces_total);
    update_entity(a_to_p_map, original_iotas_total, new_iotas_total,
        original_points_total);
    update_entity(a_to_e_map, original_iotas_total, new_iotas_total,
        original_edges_total);
    update_entity(a_to_s_map, original_iotas_total, new_iotas_total,
        original_sides_total);
  }
}

void scale_mesh(int const scale, Ume::SOA_Idx::Mesh &mesh) {
  int current_scale = 1;
  int dim = 0;

  std::cout << "Original Mesh Stats:" << std::endl;
  mesh.print_stats(std::cout);
  std::cout << std::endl;
  while (current_scale < scale) {
    dim = dim % 3;
    stitch(mesh, dim);
    current_scale *= 2;
    dim += 1;
  }
  std::cout << "Final Mesh Stats:" << std::endl;
  mesh.print_stats(std::cout);
}

bool write_mesh(char const *const basename, int const mype, int const scale,
    Ume::SOA_Idx::Mesh &mesh) {
  std::string const fname = Ume::mesh_filename(basename, scale, mype);
  std::ofstream os(fname);
  if (!os) {
    std::cerr << "Unable to open file \"" << fname << "\" for writing."
              << std::endl;
    return false;
  }
  mesh.write(os);
  os.close();
  if (!os) {
    std::cerr << "Unable to write a mesh to file \"" << fname << "\"."
              << std::endl;
    return false;
  }
  return true;
}
