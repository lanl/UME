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

/*
** Scoria Includes
*/
#if defined(USE_SCORIA) && defined(USE_CLIENT)
extern "C" {
#include "scoria.h"
}
#endif


/*
** Ume Includes
*/
#include "Ume/Comm_MPI.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/Timer.hh"
#include "Ume/gradient.hh"
#include "Ume/utils.hh"
#include "shm_allocator.hh"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#ifdef USE_CALI
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#endif

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh);
bool test_point_gathscat(Ume::SOA_Idx::Mesh &mesh);

template <class T>
void write_result(char const *const varname, int const mype, T &data);

using DBLV_T = typename Ume::DS_Types::DBLV_T;
using VEC3V_T = typename Ume::DS_Types::VEC3V_T;
using VEC3_T = typename Ume::DS_Types::VEC3_T;

int main(int argc, char *argv[]) {
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  struct client client;
  client.chatty = 0;

  scoria_init(&client);
#endif

#ifdef USE_CALI
    CALI_CXX_MARK_FUNCTION;
#endif

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

  // Do a zone-centered gradient calculation on that field (in parallel)
  if (comm.pe() == 0)
    std::cout << "Calculating gradient..." << std::endl;

  VEC3V_T pgrad, zgrad;
  Ume::Timer orig_time;
#ifdef USE_CALI
  if (comm.pe() == 0)
    std::cout << "Cali Profiling Begin..." << std::endl;
  CALI_MARK_BEGIN("Gradzatz1");
#endif
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  Ume::gradzatz(&client, mesh, zfield, zgrad, pgrad);
#else
  Ume::gradzatz(mesh, zfield, zgrad, pgrad);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz1");
#endif
  orig_time.start();
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz2");
#endif
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  Ume::gradzatz(&client, mesh, zfield, zgrad, pgrad);
#else
  Ume::gradzatz(mesh, zfield, zgrad, pgrad);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz2");
#endif
  orig_time.stop();

  // Write out results to file for validation
  write_result("point_gradient", comm.pe(), pgrad);
  write_result("zone_gradient", comm.pe(), zgrad);

  VEC3V_T pgrad_invert, zgrad_invert;
  Ume::Timer invert_time;
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert1");
#endif
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  Ume::gradzatz_invert(&client, mesh, zfield, zgrad_invert, pgrad_invert);
#else
  Ume::gradzatz_invert(mesh, zfield, zgrad_invert, pgrad_invert);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert1");
#endif
  invert_time.start();
#ifdef USE_CALI
  CALI_MARK_BEGIN("Gradzatz_Invert2");
#endif
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  Ume::gradzatz_invert(&client, mesh, zfield, zgrad_invert, pgrad_invert);
#else
  Ume::gradzatz_invert(mesh, zfield, zgrad_invert, pgrad_invert);
#endif
#ifdef USE_CALI
  CALI_MARK_END("Gradzatz_Invert2");
#endif
  invert_time.stop();

  // Write out results to file for validation
  write_result("point_gradient_invert", comm.pe(), pgrad_invert);
  write_result("zone_gradient_invert", comm.pe(), zgrad_invert);

  if (comm.pe() == 0) {
    std::cout << "Original algorithm took: " << orig_time.seconds() << "s\n";
    std::cout << "Inverted algorithm took: " << invert_time.seconds() << "s\n";
  }

  // Double check that the gradients are non-zero where we expect
#ifndef USE_CLIENT
  if (comm.pe() == 0) {
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
  UmeVector<int> grad_zones;
  for (int z = 0; z < mesh.zones.size(); ++z) {
    if (z == czi || kztyp[z] < 1)
      continue;
    if (zgrad[z] != 0.0)
      grad_zones.push_back(z);
  }

  UmeVector<int> grad_points;
  for (int p = 0; p < mesh.points.size(); ++p) {
    if (kptyp[p] > 0 && pgrad[p] != 0.0)
      grad_points.push_back(p);
  }

  std::sort(grad_zones.begin(), grad_zones.end());
  std::sort(grad_points.begin(), grad_points.end());
  UmeVector<int> diff;
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
#endif

  if (comm.pe() == 0)
    std::cout << "Done." << std::endl;
  comm.stop();

#if defined(USE_SCORIA) && defined(USE_CLIENT)
  struct request req;
  scoria_quit(&client, &req);
  scoria_wait_request(&client, &req);

  scoria_cleanup(&client);
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

template <class T>
void write_result(char const *const varname, int const mype, T &data) {
  char fname[80];
  sprintf(fname, "%s.%05d.out", varname, mype);
  std::ofstream os(fname);

  os << std::setprecision(10);

  for (auto const &val : data)
    os << val << '\n';
  os.close();
}
