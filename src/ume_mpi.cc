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
  \file ume_mpi.cc

  This is an example of MPI-based driver that reads one partition of an Ume
  binary mesh into each rank, and then performs (and tests) a gradient
  operation.

  Note that there must be as many *.ume files as there are MPI ranks, and they
  should have filenames of the form '<basename>.<pe>.ume', where <basename> is
  an arbitray string provided on the command line, and <pe> is a rank number
  with a printf format of "%05d" (zero-filled, five digits)
*/

#include "Ume/Comm_MPI.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/Timer.hh"
#include "Ume/gradient.hh"
#include "Ume/utils.hh"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh);
bool test_point_gathscat(Ume::SOA_Idx::Mesh &mesh);

using DBLV_T = typename Ume::DS_Types::DBLV_T;
using VEC3V_T = typename Ume::DS_Types::VEC3V_T;
using VEC3_T = typename Ume::DS_Types::VEC3_T;

int main(int argc, char *argv[]) {

  Ume::SOA_Idx::Mesh mesh;

  /* We need to instantiated the MPI Transport in order to get the PE number
     used to form our filename, */
  Ume::Comm::MPI comm(&argc, &argv);
  mesh.comm = &comm; // Attach the communicator to the mesh

  if (comm.pe() == 0)
    std::cout << "Initializing mesh..." << std::endl;
  /* Read the data file */
  if (!read_mesh(argv[1], comm.pe(), mesh)) {
    std::cerr << "Aborting." << std::endl;
    return 1;
  }

  /* This allows us to attach a debugger to a single rank specified in the
     UME_DEBUG_RANK environment variable. */
  Ume::debug_attach_point(comm.pe());

  /*
  if (test_point_gathscat(mesh)) {
    std::cout << comm.id() << ": test_point_gathscat PASS" << std::endl;
  } else {
    std::cout << comm.id() << ": test_point_gathscat FAIL" << std::endl;
  }
  */

  if (comm.pe() == 0)
    std::cout << "Creating zone field..." << std::endl;
  auto const &kztyp = mesh.zones.mask;

  // Find a local zone to set a value in
  int czi = mesh.zones.local_size() / 2;
  while (czi < mesh.zones.local_size() && kztyp[czi] < 1)
    czi += 1;
  assert(czi < mesh.zones.local_size());

  // Create a zone-field that is zero everywhere but in czi.
  DBLV_T zfield(mesh.zones.size(), 0.0);
  zfield[czi] = 100000.0;

  auto const &c_to_z_map = mesh.ds->caccess_intv("m:c>z");
  auto const &c_to_p_map = mesh.ds->caccess_intv("m:c>p");
  auto const &p_to_c_map = mesh.ds->caccess_intrr("m:p>rc");
  auto const &z_to_c_map = mesh.ds->caccess_intrr("m:z>c");
 
  int const num_local_corners = mesh.corners.local_size();
  int const num_local_zones = mesh.zones.local_size();
  int const num_local_points = mesh.points.local_size();
  int const num_points = mesh.points.size();

  long long int dist_c_z_map = 0;
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    dist_c_z_map += (long long int)abs(corner_idx - c_to_z_map[corner_idx]);
  }
  dist_c_z_map /= (long long int) num_local_corners;


  long long int dist_z_c_map = 0;
  for (int zone_idx = 0; zone_idx < num_local_zones; ++zone_idx) {
    long long int temp_dist_z_c_map = 0;
    long long int num_corners_this_zone = 0;
    for (int const &corner_idx : z_to_c_map[zone_idx]) {
      num_corners_this_zone++;
      temp_dist_z_c_map += (long long int)abs(zone_idx - corner_idx);
    }
    dist_z_c_map += temp_dist_z_c_map / num_corners_this_zone;
  }
  dist_z_c_map /= (long long int)num_local_zones;


  long long int dist_c_p_map = 0;
  for (int corner_idx = 0; corner_idx < num_local_corners; ++corner_idx) {
    dist_c_p_map += (long long int)abs(corner_idx - c_to_p_map[corner_idx]);
  }
  dist_c_p_map /= (long long int) num_local_corners;

  long long int dist_p_c_map = 0;
  for (int point_idx = 0; point_idx < num_local_points; ++point_idx) {
    long long int temp_dist_p_c_map = 0;
    long long int num_corners_this_point = 0;
    for (int const & corner_idx : p_to_c_map[point_idx]) {
      num_corners_this_point++;
      temp_dist_p_c_map += (long long int)abs(point_idx - corner_idx);
    }
    dist_p_c_map += temp_dist_p_c_map / num_corners_this_point;
  }
  dist_p_c_map /= (long long int)num_local_points;

  std::cout << "Rank: " << comm.pe() << "\tNum Local Points: " << num_local_points << "\tNum Local Zones: " << num_local_zones << "\tNum Local Corners: " << num_local_corners << "\tAvg. Dist C2Z Map: " << dist_c_z_map << "\tAvg. Dist C2P Map: " << dist_c_p_map << "\tAvg. Dist Z2C Map: " << dist_z_c_map << "\tAvg. Dist P2C Map: " << dist_p_c_map << std::endl;


  // Do a zone-centered gradient calculation on that field (in parallel)
  if (comm.pe() == 0)
    std::cout << "Calculating gradient..." << std::endl;

  VEC3V_T pgrad, zgrad;
  Ume::Timer orig_time;
  Ume::gradzatz(mesh, zfield, zgrad, pgrad);
  orig_time.start();
  Ume::gradzatz(mesh, zfield, zgrad, pgrad);
  orig_time.stop();

  VEC3V_T pgrad_invert, zgrad_invert;
  Ume::Timer invert_time;
  Ume::gradzatz_invert(mesh, zfield, zgrad_invert, pgrad_invert);
  invert_time.start();
  Ume::gradzatz_invert(mesh, zfield, zgrad_invert, pgrad_invert);
  invert_time.stop();

  // Double check that the gradients are non-zero where we expect
  if (comm.pe() == 0) {
    std::cout << "Original algorithm took: " << orig_time.seconds() << "s\n";
    std::cout << "Inverted algorithm took: " << invert_time.seconds() << "s\n";
    std::cout << "Checking result..." << std::endl;
  }

  if (zgrad != zgrad_invert) {
    std::cout << "PE" << mesh.mype << " zgrad != zgrad_invert" << std::endl;
    if (mesh.mype == 0) {
      for (int z = 0; z < mesh.zones.size(); ++z) {
        if (zgrad[z] != zgrad_invert[z]) {
          std::cout << "Z" << z << " " << mesh.zones.mask[z] << ": " << zgrad[z]
                    << " vs. " << zgrad_invert[z] << "\n";
        }
      }
    }
  }

  if (pgrad != pgrad_invert) {
    std::cout << "PE" << mesh.mype << " pgrad != pgrad_invert" << std::endl;
  }

  auto const &z2pz = mesh.ds->caccess_intrr("m:z>pz");
  auto const &z2p = mesh.ds->caccess_intrr("m:z>p");
  auto const &kptyp = mesh.points.mask;
  std::vector<int> grad_zones;
  for (int z = 0; z < mesh.zones.size(); ++z) {
    if (z == czi || kztyp[z] < 1)
      continue;
    if (zgrad[z] != 0.0)
      grad_zones.push_back(z);
  }

  std::vector<int> grad_points;
  for (int p = 0; p < mesh.points.size(); ++p) {
    if (kptyp[p] > 0 && pgrad[p] != 0.0)
      grad_points.push_back(p);
  }

  std::sort(grad_zones.begin(), grad_zones.end());
  std::sort(grad_points.begin(), grad_points.end());
  std::vector<int> diff;
  std::ranges::set_difference(grad_zones, z2pz[czi], std::back_inserter(diff));
  if (!diff.empty()) {
    std::cout << "PE" << mesh.mype << " zone diff " << diff.size() << " found "
              << grad_zones.size() << " expected " << z2pz.size(czi) << '\n';
  }

  diff.clear();
  std::ranges::set_difference(grad_points, z2p[czi], std::back_inserter(diff));
  if (!diff.empty()) {
    std::cout << "PE" << mesh.mype << " pt diff " << diff.size() << " found "
              << grad_points.size() << " expected " << z2p.size(czi) << '\n';
  }

  if (comm.pe() == 0)
    std::cout << "Done." << std::endl;
  comm.stop();
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
    return false;
  }
  mesh.read(is);
  is.close();
  return true;
}

bool test_point_gathscat(Ume::SOA_Idx::Mesh &mesh) {
  int const mype = mesh.comm->id();

  /* Initialize an integer field to the value 10 everywhere except:
       a copy = -1
       a source = the number of copies of this entity
     A sum will set the source to zero, which should then get scattered
     out to the copies.
  */
  int const background_val = 100;
  typename Ume::DS_Types::INTV_T int_field(mesh.points.size(), background_val);

  for (auto const &n : mesh.points.myCpys) {
    for (auto const &e : n.elements) {
      assert(int_field[e] == background_val);
      int_field[e] = -1;
    }
  }

  /* We're "pre-summing" the source counts so that we can check that the source
     values in the int_field are set to the background value. */
  std::map<int, int> src_counts;
  for (auto const &n : mesh.points.mySrcs) {
    for (auto const &e : n.elements) {
      auto [it, success] = src_counts.insert(std::make_pair(e, 0));
      it->second += 1;
    }
  }

  for (auto const &s : src_counts) {
    assert(int_field[s.first] == background_val);
    int_field[s.first] = s.second;
  }

  bool result = true;

  mesh.points.gathscat(Ume::Comm::Op::SUM, int_field);
  for (int i = 0; i < mesh.points.local_size(); ++i) {
    if (mesh.points.comm_type[i] == Ume::SOA_Idx::Entity::INTERNAL) {
      if (int_field[i] != background_val) {
        std::cout << "Rank " << mype << " expecting int_field[" << i
                  << "] == " << background_val << ", got " << int_field[i]
                  << '\n';
        result = false;
      }
    } else if (mesh.points.comm_type[i] == Ume::SOA_Idx::Entity::SOURCE ||
        mesh.points.comm_type[i] == Ume::SOA_Idx::Entity::COPY) {
      if (int_field[i] != 0) {
        std::cout << "Rank " << mype << " expecting int_field[" << i
                  << "] == 0, got " << int_field[i] << '\n';
        result = false;
      }
    }
  }

  return result;
}
