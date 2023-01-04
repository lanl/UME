/*!
  \file soa_idx_helpers.hh

  A collection of macros used to reduce boilerplate code in the implementation
  of specific mesh entity classes (e.g. Sides).
*/

#ifndef SOA_IDX_HELPERS_HH
#define SOA_IDX_HELPERS_HH 1

#include "Ume/utils.hh"
#include <cassert>
#include <iostream>

#define EQOP(N) (mesh_->ds->caccess_intv(N) == rhs.mesh_->ds->caccess_intv(N))
#define IVWRITE(N) write_bin(os, mesh_->ds->caccess_intv(N))
#define IVREAD(N) read_bin(is, mesh_->ds->access_intv(N))
#define RESIZE(N, S) (mesh_->ds->access_intv(N)).resize(S)

#define DSE_INIT_PREAMBLE(N) \
  if (init_state_ == Init_State::INITIALIZED) \
    return; \
  std::cout << N "::init_()" << std::endl; \
  assert(init_state_ != Init_State::IN_PROGRESS); \
  init_state_ = Init_State::IN_PROGRESS

#endif
