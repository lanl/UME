/*!
  \file parallel.cc

  An application for exploring parallel connectivity.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/Timer.hh"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
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
  int numpe, mype;
  Entity const *entities[NUM_ENTITIES];
  std::vector<int> send_elements[NUM_ENTITIES];
  std::vector<int> recv_elements[NUM_ENTITIES];

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
void build_comm_entities(CONNS &conns, int const entity);
void graph_1TO1(
    char const *const filename, MAP1TO1 const &all_recvs, bool const volelem);
void lookat_subset_ghosts(CONNS const &conns, int const entity);
/****************************************************************************/

int main(int argc, char const *const argv[]) {
  std::vector<Mesh> ranks{read_meshes(argc, argv)};
  if (ranks.empty())
    return 1;
  std::vector<ConnMesh> conn_ranks(ranks.begin(), ranks.end());

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

  Ume::Timer build_timer;
  build_timer.start();
  build_comm_entities(conn_ranks, ConnMesh::ZONES);
  build_timer.stop();
  std::cout << "build_comm_entities took: " << build_timer << '\n';
  build_timer.clear();
  for (auto const &c : conn_ranks) {
    auto const &csends = c.send_elements[ConnMesh::ZONES];
    const size_t ns = csends.size();
    auto const &crecvs = c.recv_elements[ConnMesh::ZONES];
    const size_t nr = crecvs.size();
    const size_t nz = c.entities[ConnMesh::ZONES]->size();

    std::cout << "PE" << c.mype << " has (" << nr << "r + " << ns << "s =) "
              << nr + ns << "/" << nz << " communicating zones" << std::endl;
    for (auto const &s : c.entities[ConnMesh::ZONES]->subsets) {
      build_timer.start();
      std::vector<int> sorted{s.elements};
      std::sort(sorted.begin(), sorted.end());

      std::vector<int> srecvs;
      std::set_intersection(crecvs.begin(), crecvs.end(), sorted.begin(),
          sorted.end(), std::back_inserter(srecvs));

      std::vector<int> ssends;
      std::set_intersection(csends.begin(), csends.end(), sorted.begin(),
          sorted.end(), std::back_inserter(ssends));
      build_timer.stop();

      std::cout << "\t" << s.name << " has " << sorted.size() << " zones, ("
                << srecvs.size() << "r + " << ssends.size() << "s =) "
                << srecvs.size() + ssends.size() << " communicating "
                << build_timer << "\n";
      build_timer.clear();
    }
  }

  lookat_subset_ghosts(conn_ranks, ConnMesh::ZONES);
  graph_1TO1("z_conn.dot", all_z_recvs, true);
  graph_1TO1("p_conn.dot", all_p_recvs, false);
  return 0;
}

void lookat_subset_ghosts(CONNS const &conns, int const entity) {
  for (auto const &c : conns) {
    std::cout << "PE" << c.mype << " Subset ghosts" << '\n';
    for (auto const &s : c.entities[entity]->subsets) {
      const int elocal = c.entities[entity]->lsize;
      size_t total = 0, ent_ghost = 0;
      for (size_t i = s.lsize; i < s.elements.size(); ++i) {
        total += 1;
        if (s.elements[i] >= elocal)
          ent_ghost += 1;
      }
      std::cout << '\t' << s.name << ' ' << ent_ghost << '/' << total
                << " subset ghosts are entity ghosts\n";
    }
  }
}

void graph_1TO1(
    char const *const filename, MAP1TO1 const &all_recvs, bool const volelem) {
  std::map<int, std::map<int, size_t>> send_summary;
  for (auto const &recvr : all_recvs) {
    int sendpe = recvr.second.pe;
    int recvpe = recvr.first.pe;
    auto [it, status] = send_summary[sendpe].insert(std::make_pair(recvpe, 0));
    it->second += 1;
  }
  std::ofstream os(filename);
  std::string conn;
  if (volelem) {
    os << "strict graph {\n";
    conn = " -- ";
  } else {
    os << "digraph {\n";
    conn = " -> ";
  }

  for (auto const &s : send_summary) {
    for (auto const &r : s.second) {
      os << s.first << conn << r.first << " [ label = " << r.second << "];\n";
    }
  }
  os << "}\n";
  os.close();
}

ConnMesh::ConnMesh(Mesh const &m) : numpe{m.numpe}, mype{m.mype} {
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

void build_comm_entities(CONNS &conns, int const entity) {
  std::map<int, std::set<int>> sends, recvs;
  for (auto const &c : conns) {
    std::vector<int> const &cpy_idx = c.entities[entity]->cpy_idx;
    std::vector<int> const &src_pe = c.entities[entity]->src_pe;
    std::vector<int> const &src_idx = c.entities[entity]->src_idx;

    size_t const N = cpy_idx.size();
    for (size_t i = 0; i < N; ++i) {
      recvs[c.mype].insert(cpy_idx[i]);
      sends[src_pe[i]].insert(src_idx[i]);
    }
  }

  for (auto &c : conns) {
    c.send_elements[entity].assign(sends[c.mype].begin(), sends[c.mype].end());
    c.recv_elements[entity].assign(recvs[c.mype].begin(), recvs[c.mype].end());
  }
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
