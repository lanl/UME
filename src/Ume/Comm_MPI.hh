/*!
\file Ume/Comm_MPI.hh
*/

#ifndef COMM_MPI_HH
#define COMM_MPI_HH 1

#include "Ume/Comm_Transport.hh"
#include <unordered_map>

namespace Ume {
namespace Comm {

class MPI : public Transport {
public:
  MPI(int *argc, char ***argv, int virtual_rank);
  int stop() override;

private:
  int virtual_rank_;
  int rank_;
  int numpe_;
  std::unordered_map<int, int> v2r_rank_; //! virtual-to-real rank map
};

} // namespace Comm
} // namespace Ume

#endif
