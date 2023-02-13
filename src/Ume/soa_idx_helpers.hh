/*!
  \file soa_idx_helpers.hh

  A collection of macros used to reduce boilerplate code in the implementation
  of specific mesh entity classes (e.g. Sides).
*/

#ifndef SOA_IDX_HELPERS_HH
#define SOA_IDX_HELPERS_HH 1

#define TRACE_INIT 0

#include "Ume/utils.hh"
#include <cassert>

#define EQOP(N) (ds().caccess_intv(N) == rhs.ds().caccess_intv(N))
#define IVWRITE(N) write_bin(os, ds().caccess_intv(N))
#define IVREAD(N) read_bin(is, ds().access_intv(N))
#define RESIZE(N, S) (ds().access_intv(N)).resize(S)

#if TRACE_INIT
#include <iostream>
#define DSE_INIT_PREAMBLE(N) \
  if (init_state_ == Init_State::INITIALIZED) \
    return false; \
  init_depth(1); \
  if (mesh().comm->id() == 0) \
    std::cout << "PE0: " << std::string(init_depth(0) - 1, '\t') \
              << N "::init_()" << std::endl; \
  assert(init_state_ != Init_State::IN_PROGRESS); \
  init_state_ = Init_State::IN_PROGRESS

#define DSE_INIT_EPILOGUE \
  init_state_ = Init_State::INITIALIZED; \
  init_depth(-1); \
  return true;
#else
#define DSE_INIT_PREAMBLE(N) \
  if (init_state_ == Init_State::INITIALIZED) \
    return false; \
  assert(init_state_ != Init_State::IN_PROGRESS); \
  init_state_ = Init_State::IN_PROGRESS

#define DSE_INIT_EPILOGUE \
  init_state_ = Init_State::INITIALIZED; \
  return true;
#endif
#endif
