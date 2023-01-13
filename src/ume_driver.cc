/*!
  \file ume.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include <fstream>
#include <iostream>
#include <vector>

using namespace Ume::SOA_Idx;

std::vector<Mesh> read_meshes(int const argc, char const *const argv[]);

int main(int argc, char const *const argv[]) {
  std::vector<Mesh> ranks{read_meshes(argc, argv)};
  if (ranks.empty())
    return 1;

  auto const &test = ranks[0].ds->caccess_vec3v("corner_csurf");
  auto const &test2 = ranks[0].ds->caccess_vec3v("side_surz");

  return 0;
}

std::vector<Mesh> read_meshes(int const argc, char const *const argv[]) {
  std::vector<Mesh> ranks;
  ranks.resize(argc - 1);
  bool need_sort{false};
  for (int i = 1; i < argc; ++i) {
    std::cout << "Reading: " << argv[i] << '\n';
    std::ifstream is(argv[i]);
    if (!is) {
      std::cerr << "Unable to open file \"" << argv[i] << "\" for reading."
                << std::endl;
      return std::vector<Mesh>{};
    }
    ranks[i - 1].read(is);
    if (ranks[i - 1].mype != i - 1)
      need_sort = true;
  }
  size_t const numpe = static_cast<size_t>(ranks[0].numpe);
  if (numpe != ranks.size()) {
    std::cerr << "Warning: the initial mesh had " << numpe
              << " ranks, but only " << ranks.size() << " were read"
              << std::endl;
  }
  if (need_sort) {
    std::cerr << "Warning: sorting input ranks" << std::endl;
    std::sort(ranks.begin(), ranks.end(),
        [](Mesh const &a, Mesh const &b) { return a.mype < b.mype; });
  }

  return ranks;
}
