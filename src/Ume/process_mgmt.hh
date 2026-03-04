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
\file Ume/process_mgmt.hh
*/

#ifndef UME_PROCESS_MGMT_HH
#define UME_PROCESS_MGMT_HH

#include <Kokkos_Core.hpp>

namespace Ume {

void initialize(int &argc, char *argv[]);
void finalize();
extern "C" void error_stop(char const msg[]);

}

#endif
