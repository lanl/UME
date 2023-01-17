/*!
\file Comm_MPI.cc
*/

#ifdef HAVE_MPI

#include "Ume/Comm_MPI.hh"
#include <mpi.h>
#include <vector>

namespace Ume {
namespace Comm {

MPI::MPI(int *argc, char ***argv, int const virtual_rank)
    : virtual_rank_{virtual_rank} {
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &numpe_);
  std::vector<int> r2v(numpe_, -1);
  r2v[rank_] = virtual_rank_;
  MPI_Allgather(MPI_IN_PLACE, numpe_, MPI_INT, r2v.data(), numpe_, MPI_INT,
      MPI_COMM_WORLD);
  for (int i = 0; i < numpe_; ++i) {
    v2r_rank_.insert(std::make_pair(r2v[i], i));
  }
}

int MPI::stop() {
  MPI_Finalize();
  return 0;
}

} // namespace Comm
} // namespace Ume

#endif
