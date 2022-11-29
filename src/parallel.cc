/*!
  \file parallel.cc

  An application for exploring parallel connectivity.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

using namespace Ume::SOA_Idx;

/* A global index tuple of PE rank and array element index */
struct PEIdx {
  int pe, idx;
  constexpr bool operator==(PEIdx const &rhs) const {
    return pe == rhs.pe && idx == rhs.idx;
  }
  constexpr bool operator<(PEIdx const &rhs) const {
    return pe < rhs.pe || (pe == rhs.pe && idx < rhs.idx);
  }
};

std::ostream &operator<<(std::ostream &os, PEIdx const &pi) {
  return os << '(' << pi.pe << ',' << pi.idx << ')';
}

/* A simplification of the full SOA_Idx_Mesh that only contains the Entities of
   the source mesh.  This is useful for making generic connectivity analyzers,
   as the Entities are available as array indices, rather than as class member
   names. */
struct ConnMesh {
public:
  enum EntType { CORNERS, EDGES, FACES, POINTS, SIDES, ZONES, NUM_ENTITIES };
  size_t numpe, mype;
  Entity const *entities[NUM_ENTITIES];

public:
  explicit ConnMesh(Mesh const &m);
};

// Type for 1-to-1 mapping between PEIdx
using MAP1TO1 = std::map<PEIdx, PEIdx>;
// Type for 1-to-N mapping between PEIdx
using MAP1TON = std::map<PEIdx, std::set<PEIdx>>;
using CONNS = std::vector<ConnMesh>;

std::vector<Mesh> read_meshes(int const argc, char const *const argv[]);
MAP1TO1 build_all_recvs(CONNS const &conns, int const entity);
bool compare_all(MAP1TO1 const &all_recvs, CONNS const &conns, int const ent);

int main(int argc, char const *const argv[]) {
  std::vector<Mesh> ranks{read_meshes(argc, argv)};
  if (ranks.empty())
    return 1;
  std::vector<ConnMesh> conn_ranks(ranks.begin(), ranks.end());
  size_t const numpe = static_cast<size_t>(ranks[0].numpe);

  MAP1TO1 all_c_recvs{build_all_recvs(conn_ranks, ConnMesh::CORNERS)};
  MAP1TO1 all_e_recvs{build_all_recvs(conn_ranks, ConnMesh::EDGES)};
  MAP1TO1 all_f_recvs{build_all_recvs(conn_ranks, ConnMesh::FACES)};
  MAP1TO1 all_p_recvs{build_all_recvs(conn_ranks, ConnMesh::POINTS)};
  MAP1TO1 all_s_recvs{build_all_recvs(conn_ranks, ConnMesh::SIDES)};
  MAP1TO1 all_z_recvs{build_all_recvs(conn_ranks, ConnMesh::ZONES)};

  std::cout << "Corners "
            << compare_all(all_c_recvs, conn_ranks, ConnMesh::CORNERS) << '\n';
  std::cout << "Edges " << compare_all(all_e_recvs, conn_ranks, ConnMesh::EDGES)
            << '\n';
  std::cout << "Faces " << compare_all(all_f_recvs, conn_ranks, ConnMesh::FACES)
            << '\n';
  std::cout << "Points "
            << compare_all(all_p_recvs, conn_ranks, ConnMesh::POINTS) << '\n';
  std::cout << "Sides " << compare_all(all_s_recvs, conn_ranks, ConnMesh::SIDES)
            << '\n';
  std::cout << "Zones " << compare_all(all_z_recvs, conn_ranks, ConnMesh::ZONES)
            << '\n';

  return 0;
}

ConnMesh::ConnMesh(Mesh const &m)
    : numpe{static_cast<size_t>(m.numpe)}, mype{static_cast<size_t>(m.mype)} {
  entities[CORNERS] = &(m.corners);
  entities[EDGES] = &(m.edges);
  entities[FACES] = &(m.faces);
  entities[POINTS] = &(m.points);
  entities[SIDES] = &(m.sides);
  entities[ZONES] = &(m.zones);
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

MAP1TO1 build_all_recvs(CONNS const &conns, int const entity) {
  MAP1TO1 all_recvs;
  for (auto const &c : conns) {
    PEIdx dst;
    dst.pe = c.mype;

    std::vector<int> const &cpy_idx = c.entities[entity]->cpy_idx;
    std::vector<int> const &src_pe = c.entities[entity]->src_pe;
    std::vector<int> const &src_idx = c.entities[entity]->src_idx;

    size_t const N = cpy_idx.size();
    for (size_t i = 0; i < N; ++i) {
      if (src_pe[i] == c.mype)
        continue;
      dst.idx = cpy_idx[i];
      all_recvs[dst] = PEIdx{src_pe[i], src_idx[i]};
    }
  }
  return all_recvs;
}

bool compare_all(MAP1TO1 const &all_recvs, CONNS const &conns, int const ent) {
  /* Invert the all_recvs map.  Note that while receives are one-to-one, sends
     can be one-to-many. */
  MAP1TON all_sends;
  for (auto const &e : all_recvs) {
    all_sends[e.second].insert(e.first);
  }

  size_t const numpe = conns[0].numpe;

  /* For each PE, make a unique set of sending elements from our all_sends map
   */
  std::vector<std::set<int>> my_sends_by_pe{numpe};
  for (auto const &p : all_sends) {
    my_sends_by_pe[p.first.pe].insert(p.first.idx);
  }

  /* Build a similar list, but from the Entity sendTo list */
  std::vector<std::set<int>> orig_sends_by_pe{numpe};
  for (auto const &c : conns) {
    for (auto const &s : c.entities[ent]->sendTo) {
      orig_sends_by_pe[c.mype].insert(s.elements.begin(), s.elements.end());
    }
  }
  bool the_same = (orig_sends_by_pe == my_sends_by_pe);

  if (!the_same) {
    std::cout << "Compare FAILED on ent " << ent << '\n';
    for (size_t i = 0; i < numpe; ++i) {
      std::cout << "PE" << i << " orig_sends: " << orig_sends_by_pe[i].size()
                << " new_sends: " << my_sends_by_pe[i].size() << '\n';
    }
  }

  return the_same;
}
