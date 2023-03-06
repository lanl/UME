/*!
  \file ume_serial.cc

  This is an example of a serial driver for Ume.  It simply reads one or more
  binary Ume file(s) and accesses some computed variables in the first one.
*/

/*
** Scoria Includes
*/
#ifdef USE_SCORIA
extern "C" {
#include "client.h"
#include "config.h"

#include "client_cleanup.h"
#include "client_init.h"
#include "client_memory.h"
#include "client_wait_requests.h"

#include "shm_malloc.h"
}
#endif /* USE_SCORIA */

/*
** Ume Includes
*/
#include "Ume/SOA_Idx_Mesh.hh"
#include "shm_allocator.hh"
#include <fstream>
#include <iostream>
#include <vector>

using namespace Ume::SOA_Idx;

UmeVector<Mesh> read_meshes(int argc, char *argv[]);

int main(int argc, char *argv[]) {
#ifdef USE_SCORIA
  struct client client;
  client.chatty = 0;

  init(&client);
#endif /* USE_SCORIA */

  UmeVector<Mesh> ranks{read_meshes(argc, argv)};

  if (ranks.empty())
    return 1;

  Ume::Comm::Dummy_Transport comm;
  ranks[0].comm = &comm;

  auto const &test = ranks[0].ds->caccess_vec3v("corner_csurf");
  auto const &test2 = ranks[0].ds->caccess_vec3v("side_surz");
  auto const &test3 = ranks[0].ds->caccess_vec3v("point_norm");

#ifdef USE_SCORIA
  struct request req;
  scoria_quit(&client, &req);
  wait_request(&client, &req);

  cleanup(&client);
#endif /* USE_SCORIA */

  return 0;
}

UmeVector<Mesh> read_meshes(int const argc, char *argv[]) {
  UmeVector<Mesh> ranks;
  ranks.resize(argc - 1);
  bool need_sort{false};
  if (argc == 1) {
    std::cerr << "Usage: ume_serial <ume file>+" << std::endl;
    std::exit(1);
  }
  for (int i = 1; i < argc; ++i) {
    std::cout << "Reading: " << argv[i] << '\n';
    std::ifstream is(argv[i]);
    if (!is) {
      std::cerr << "Unable to open file \"" << argv[i] << "\" for reading."
                << std::endl;
      return UmeVector<Mesh>{};
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
