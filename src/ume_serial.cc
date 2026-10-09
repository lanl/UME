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
  \file ume_serial.cc

  This is an example of a serial driver for Ume.  It simply reads one or more
  binary Ume file(s) and accesses some computed variables in the first one.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/process_mgmt.hh"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

using namespace Ume::SOA_Idx;

using Mesh_Ptr = std::unique_ptr<Mesh>;

std::vector<Mesh_Ptr> read_meshes(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cerr << "Usage: ume_serial <ume file>+" << std::endl;
    return 1;
  }

  /* The derived mesh variables are computed with Kokkos kernels, so Kokkos has
     to be up before anything touches the Datastore.  This also strips the
     --kokkos-* arguments before the file names are read. */
  Ume::initialize(argc, argv);
  int status = 0;
  {
    std::vector<Mesh_Ptr> ranks{read_meshes(argc, argv)};
    if (ranks.empty()) {
      status = 1;
    } else {
      Ume::Comm::Dummy_Transport comm;
      ranks[0]->comm = &comm;

      [[maybe_unused]] auto const &test =
          ranks[0]->ds->caccess_vec3v("corner_csurf");
      [[maybe_unused]] auto const &test2 =
          ranks[0]->ds->caccess_vec3v("side_surz");
      [[maybe_unused]] auto const &test3 =
          ranks[0]->ds->caccess_vec3v("point_norm");
    }
  }
  Ume::finalize();
  return status;
}

/*! A Mesh cannot be moved (see the note on Mesh's deleted move constructor),
    so the meshes are held by pointer: sorting the ranks below then permutes
    pointers and leaves every Mesh at the address it was constructed at. */
std::vector<Mesh_Ptr> read_meshes(int const argc, char *argv[]) {
  std::vector<Mesh_Ptr> ranks;
  bool need_sort{false};
  /* Reachable even though main() checks argc too: Ume::initialize() takes argc
     by reference and strips the --kokkos-* arguments, so `ume_serial
     --kokkos-num-threads=4` arrives here with nothing left.  Return rather than
     std::exit, which would skip the Ume::finalize() main() is structured to
     reach; an empty result is already how main() reports failure. */
  if (argc == 1) {
    std::cerr << "Usage: ume_serial <ume file>+" << std::endl;
    return ranks;
  }
  for (int i = 1; i < argc; ++i) {
    std::cout << "Reading: " << argv[i] << '\n';
    std::ifstream is(argv[i]);
    if (!is) {
      std::cerr << "Unable to open file \"" << argv[i] << "\" for reading."
                << std::endl;
      return std::vector<Mesh_Ptr>{};
    }
    ranks.push_back(std::make_unique<Mesh>());
    ranks.back()->read(is);
    if (!is) {
      std::cerr << "Unable to read a mesh from file \"" << argv[i] << "\"."
                << std::endl;
      return std::vector<Mesh_Ptr>{};
    }
    if (ranks.back()->mype != i - 1)
      need_sort = true;
  }
  size_t const numpe = static_cast<size_t>(ranks[0]->numpe);
  if (numpe != ranks.size()) {
    std::cerr << "Warning: the initial mesh had " << numpe
              << " ranks, but only " << ranks.size() << " were read"
              << std::endl;
  }
  if (need_sort) {
    std::cerr << "Warning: sorting input ranks" << std::endl;
    std::sort(ranks.begin(), ranks.end(),
        [](Mesh_Ptr const &a, Mesh_Ptr const &b) { return a->mype < b->mype; });
  }

  return ranks;
}
