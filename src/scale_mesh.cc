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

  This is an example of MPI-based driver that reads one partition of an Ume
  binary mesh into each rank, and then performs (and tests) a gradient
  operation.

  Note that there must be as many *.ume files as there are MPI ranks, and they
  should have filenames of the form '<basename>.<pe>.ume', where <basename> is
  an arbitray string provided on the command line, and <pe> is a rank number
  with a printf format of "%05d" (zero-filled, five digits)
*/

#include "Ume/Comm_MPI.hh"
#include "Ume/DS_Types.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh);

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

  Ume::SOA_Idx::Mesh mesh;

  int mype = 0;
#ifdef HAVE_MPI
  Ume::Comm::MPI comm(&argc, &argv);
  mesh.comm = &comm;

  mype = comm.pe();
#endif
  if (mype == 0)
    std::cout << "Initializing mesh..." << std::endl;

  if (!read_mesh(argv[1], mype, mesh)) {
    std::cerr << "Aborting." << std::endl;
    return 1;
  }

  int scale = atoi(argv[2]);
  if ((scale == 0) || ((scale & (scale - 1)) != 0)) {
    std::cerr << "Scale must be a power of 2" << std::endl;
    return 1;
  }

  if (mype == 0)
    std::cout << "Scaling mesh by a factor of " << scale << "..." << std::endl;

  scale_mesh(scale, mesh);

  if (mype == 0)
    std::cout << "Writing scaled mesh..." << std::endl;

  if (!write_mesh(argv[1], mype, scale, mesh)) {
    std::cerr << "Aborting." << std::endl;
    return 1;
  }

  if (mype == 0)
    std::cout << "Done." << std::endl;
#ifdef HAVE_MPI
  comm.stop();
#endif
  return 0;
}

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh) {
  char fname[80];
  sprintf(fname, "%s.%05d.ume", basename, mype);
  std::ifstream is(fname);
  if (!is) {
    std::cerr << "Unable to open file \"" << fname << "\" for reading."
              << std::endl;
  }
  mesh.read(is);
  is.close();
  return true;
}

double get_bounding_dim(Ume::SOA_Idx::Mesh &mesh, int dim) {
  auto const &pcoord = mesh.ds->caccess_vec3v("pcoord");

  double num_points = mesh.points.size();
  double p_max = 0;
  for (int p = 0; p < num_points; ++p) {
    double x = pcoord[p][dim];

    if (x > p_max)
      p_max = x;
  }

  return p_max;
}

void double_entity_count(Ume::SOA_Idx::Entity &entity) {
  int original_local = entity.local_size();
  int original_total = entity.size();
  int original_ghost = original_total - original_local;

  int new_local = original_local * 2;
  int new_total = original_total * 2;
  int new_ghost = original_ghost * 2;
  entity.resize(new_local, new_total, new_ghost);
}

void update_coords(Ume::DS_Types::VEC3V_T &coords, const int iter_start,
    const int iter_end, const double delta, const int dim) {
  for (int c = iter_start; c < iter_end; ++c) {
    coords[c][dim] = coords[c - iter_start][dim] + delta;
  }
}

void update_entity(Ume::DS_Types::INTV_T &map, const int iter_start,
    const int iter_end, const int delta) {
  for (int e = iter_start; e < iter_end; ++e) {
    map[e] = map[e - iter_start] + delta;
  }
}

void stitch(Ume::SOA_Idx::Mesh &mesh, const int dim) {
  //! ----- Points -----
  // Double Number of Points
  double p_max = get_bounding_dim(mesh, dim);

  int original_points_total = mesh.points.size();
  int new_points_total = original_points_total * 2;
  double_entity_count(mesh.points);

  // Update Point Coordinates of new Points
  auto &new_pcoords = mesh.ds->access_vec3v("pcoord");
  update_coords(
      new_pcoords, original_points_total, new_points_total, p_max, dim);

  //! ----- Zones -----
  // Double Number of Zones
  int original_zones_total = mesh.zones.size();
  //int new_zones_total = original_zones_total * 2;
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
  char fname[80];
  sprintf(fname, "%s.%05d.%05d.ume", basename, scale, mype);
  std::ofstream os(fname);
  if (!os) {
    std::cerr << "Unable to open file \"" << fname << "\" for reading."
              << std::endl;
  }
  mesh.write(os);
  os.close();
  return true;
}
