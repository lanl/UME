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
  \file Ume/utils.cc
*/

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

namespace Ume {

void debug_attach_point(int const mype) {
  int release = 0;
  char const *const envvalue = std::getenv("UME_DEBUG_RANK");
  if (envvalue == nullptr)
    return;
  int const stoppe = std::stoi(envvalue, nullptr, 10);
  if (stoppe == mype) {
    std::cerr << "Execution is paused on rank " << stoppe << " PID " << getpid()
              << " because the environment variable\nUME_DEBUG_RANK is set. "
              << "If you are debugging a segfault, add the\n"
              << "\"-mca orte_abort_on_non_zero_status 0\" option to mpirun.\n"
              << "In gdb, go 'up' to the the debug_attach_point frame, and do\n"
              << "`setvar release = 1`\n"
              << std::endl;

    while (!release) {
      sleep(5);
    }
  }
#ifdef HAVE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
}

int init_depth(int const delta) {
  static int depth = 0;
  return depth += delta;
}

} // namespace Ume
