/*!
  \file ume_mpi.cc
*/

#include "Ume/Comm_MPI.hh"
#include "Ume/SOA_Idx_Mesh.hh"
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

  DBLV_T zfield(mesh.zones.size(), 0.0);
  zfield[mesh.zones.lsize / 2] = 100000.0;

  VEC3V_T pgrad, zgrad;
  Ume::gradzatz(mesh, zfield, pgrad, zgrad);

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
  for (int i = 0; i < mesh.points.lsize; ++i) {
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
