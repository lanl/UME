/*!
  \file ume_mpi.cc
*/

#include "Ume/Comm_MPI.hh"
#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/utils.hh"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

bool read_mesh(
    char const *const basename, int const mype, Ume::SOA_Idx::Mesh &mesh);

int main(int argc, char *argv[]) {
  Ume::SOA_Idx::Mesh mesh;
  Ume::Comm::MPI comm(&argc, &argv);
  if (!read_mesh(argv[1], comm.pe(), mesh)) {
    std::cerr << "Aborting." << std::endl;
    return 1;
  }
  Ume::debug_attach_point(comm.pe());
  mesh.comm = &comm;
  auto const &test = mesh.ds->caccess_vec3v("corner_csurf");
  auto const &test2 = mesh.ds->caccess_vec3v("side_surz");
  auto const &test3 = mesh.ds->caccess_vec3v("point_norm");

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
