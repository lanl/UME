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
  \file Ume/soa_idx_helpers.hh

  A collection of macros used to reduce boilerplate code in the implementation
  of specific mesh entity classes (e.g. Sides).
*/

#ifndef UME_SOA_IDX_HELPERS_HH
#define UME_SOA_IDX_HELPERS_HH 1

#define TRACE_INIT 1

#include "Ume/utils.hh"
#include <cassert>

#define EQOP(N) (ds().caccess_intv(N) == rhs.ds().caccess_intv(N))
#define IVWRITE(N) write_bin(os, ds().caccess_intv(N))
#define IVREAD(N) read_bin(is, ds().access_intv(N))
#define RESIZE(N, S) (ds().access_intv(N)).resize(S)

#if TRACE_INIT
#include <iostream>
#define VAR_INIT_PREAMBLE(N) \
  if (init_state_ == Init_State::INITIALIZED) \
    return false; \
  init_depth(1); \
  if (mesh().comm->id() == 0) \
    std::cout << "PE0: " << std::string(init_depth(0) - 1, '\t') \
              << N "::init_()" << std::endl; \
  assert(init_state_ != Init_State::IN_PROGRESS); \
  init_state_ = Init_State::IN_PROGRESS

#define VAR_INIT_EPILOGUE \
  init_state_ = Init_State::INITIALIZED; \
  init_depth(-1); \
  return true;
#else
#define VAR_INIT_PREAMBLE(N) \
  if (init_state_ == Init_State::INITIALIZED) \
    return false; \
  assert(init_state_ != Init_State::IN_PROGRESS); \
  init_state_ = Init_State::IN_PROGRESS

#define VAR_INIT_EPILOGUE \
  init_state_ = Init_State::INITIALIZED; \
  return true;
#endif
#endif
