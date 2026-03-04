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
\file Ume/process_mgmt.cc
*/

#include "process_mgmt.hh"
#include "memory.hh"

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif

namespace {

void show_backtrace() {
#ifdef NO_LIBUNWIND
  printf("No libunwind backtrace available on this platform.\n");
#else
#endif
}

void halt_with_backtrace(char const msg[], bool const show_bt) {
  printf("\n%s\n", msg);
  if (show_bt)
    show_backtrace();

#ifdef HAVE_MPI
  int err_code = MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  if (err_code)
    std::exit(EXIT_FAILURE);
#else
  std::exit(EXIT_FAILURE);
#endif

}

}

namespace Ume {

/* First initialize Kokkos, then the memory pool */
void initialize(int &argc, char *argv[]) {
  Kokkos::initialize(argc, argv);

  if (!GetMemPool().IsInitialized()) {
    GetMemPool().IsInitialized() = true;

    GetMemPool().IsPoolEnabled() = true;

    /* Set pool defaults if environment variables are not set */
    unsigned blockSizeBytes = 128;
    size_t defaultSizeMB = 8000;
    const char *envPoolSizeMB = std::getenv("MEMORY_POOL_SIZE_MB");
    size_t poolSizeBytes;

    if (envPoolSizeMB != nullptr)
      poolSizeBytes = std::atoi(envPoolSizeMB);
    else
      poolSizeBytes = defaultSizeMB;

    poolSizeBytes *= 1024 * 1024;
    GetMemPool().Pool() =
        MemoryPoolAllocation<DefaultMemSpace>(poolSizeBytes, blockSizeBytes);
  }
}

/* First finalize the memory pool, then Kokkos */
void finalize() {
  GetMemPool().Pool().Finalize();
  Kokkos::finalize();
}

/* With error condition and backtrace, abort the job (if have MPI)
 * or process exit (if no MPI). This is an asynchronous operation:
 * all calling processes will write to standard out. */
void error_stop(char const msg[]) {
  finalize();
  halt_with_backtrace(msg, true);
}

} // namespace Ume
