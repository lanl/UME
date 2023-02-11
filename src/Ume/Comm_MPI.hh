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
  MPI(int *argc, char ***argv);
  void set_virtual_rank(int const virtual_rank);
  void exchange(Buffers<DS_Types::INTV_T> const &sends,
      Buffers<DS_Types::INTV_T> &recvs) override;
  void exchange(Buffers<DS_Types::DBLV_T> const &sends,
      Buffers<DS_Types::DBLV_T> &recvs) override;
  void exchange(Buffers<DS_Types::VEC3V_T> const &sends,
      Buffers<DS_Types::VEC3V_T> &recvs) override;
  int stop() override;
  void abort(char const *const message) override;

  int get_tag();
  //! Translate from virtual PE to real PE
  /*! The "virtual PE" is the PE identifier that is loaded from the Ume data
      files.  As this may not match the layout of PEs in MPI, we provide a
      translation service. */
  int translate_pe(int const virtual_pe) const {
    if (use_virtual_ranks_) {
      if (auto res = v2r_rank_.find(virtual_pe); res != v2r_rank_.end()) {
        return res->second;
      }
      return -1;
    }
    return virtual_pe;
  }
  constexpr int pe() const { return rank_; }
  constexpr int numpe() const { return numpe_; }
  int id() const override { return rank_; }

private:
  bool use_virtual_ranks_;
  std::unordered_map<int, int> v2r_rank_; //! virtual-to-real rank map
  int rank_;
  int numpe_;
  int max_tag_;
};

} // namespace Comm
} // namespace Ume

#endif
