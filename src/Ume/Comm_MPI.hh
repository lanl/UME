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
\file Ume/Comm_MPI.hh
*/

#ifndef UME_COMM_MPI_HH
#define UME_COMM_MPI_HH 1

#include "Ume/Comm_Transport.hh"
#include <unordered_map>

namespace Ume {
namespace Comm {

//! An MPI-based Transport mechanism
class MPI : public Transport {
public:
  MPI(int *argc, char ***argv);

  //! Set the rank that the partitioned mesh *thinks* it is
  /*! This could be used to map between virtual ranks and actual rank numbers.
      If you don't call this, virtual rank mapping is not used. */
  void set_virtual_rank(int const virtual_rank);
  void exchange(Buffers<DS_Types::INTV_T> const &sends,
      Buffers<DS_Types::INTV_T> &recvs) override;
  void exchange(Buffers<DS_Types::DBLV_T> const &sends,
      Buffers<DS_Types::DBLV_T> &recvs) override;
  void exchange(Buffers<DS_Types::VEC3V_T> const &sends,
      Buffers<DS_Types::VEC3V_T> &recvs) override;
  int stop() override;
  void abort(char const *const message) override;

  //! Return a new MPI_Tag value
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
  std::unordered_map<int, int> v2r_rank_; //!< virtual-to-real rank map
  int rank_;
  int numpe_;
  int max_tag_; //!< The maximum allowable tag value
};

} // namespace Comm
} // namespace Ume

#endif
