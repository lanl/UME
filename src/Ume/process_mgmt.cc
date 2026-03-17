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

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif
#ifndef __CUDACC__
#ifndef NO_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif
#endif

static bool ume_is_initialized = false;
static bool ume_is_finalized = false;

namespace {
extern "C" {

void show_backtrace() {
#ifndef __CUDACC__
#ifdef NO_LIBUNWIND
  printf("No libunwind backtrace available on this platform.\n");
#else
  /* Limit on the number of stack frames output.
   * Set to a large number to disable. */
  constexpr int max_stack_frames = 0xFFFF;
  constexpr int namelen = 512;

  unw_context_t uc;
  if (unw_getcontext(&uc)) {
    printf("<stack trace unavailable>\n");
    return;
  }

  unw_cursor_t cursor;
  if (unw_init_local(&cursor, &uc)) {
    printf("<stack trace unavailable>\n");
    return;
  }

  unw_word_t ip, sp, offset;
  int stack_frame_count = 0, name_status, step_status;
  char procname[namelen];
  do {
    if (stack_frame_count < max_stack_frames) {
      name_status = unw_get_proc_name(&cursor, procname, namelen, &offset);
      if (name_status) {
        printf("%2d: <no information>\n", stack_frame_count);
      } else {
        if (unw_get_reg(&cursor, UNW_REG_IP, &ip))
          ip = 0xDEADBEEF;
        if (unw_get_reg(&cursor, UNW_REG_SP, &sp))
          sp = 0xDEADBEEF;
        printf("%2d: ip = %#010lx, sp = %#014lx: %s\n", stack_frame_count,
            (unsigned long)ip, (unsigned long)sp, procname);
      }
    }
    stack_frame_count += 1;
  } while ((step_status = unw_step(&cursor)) > 0);
  if (step_status < 0) {
    printf("<stack trace truncated by libunwind error>\n\t%s\n",
        unw_strerror(step_status));
  } else if (stack_frame_count >= max_stack_frames) {
    printf("<stack trace output truncated at %d/%d frames>\n", max_stack_frames,
        stack_frame_count);
  }
#endif
#endif
}

void halt_with_backtrace(char const msg[], bool const show_bt) {
  printf("\n%s\n", msg);
  if (show_bt)
    show_backtrace();

#ifdef HAVE_MPI
  int mpi_is_initialized, mpi_is_finalized, err = 0;
  MPI_Initialized(&mpi_is_initialized);
  MPI_Finalized(&mpi_is_finalized);

  if (mpi_is_initialized && !mpi_is_finalized) {
    err = MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    if (err)
      std::exit(EXIT_FAILURE);
  } else
    std::exit(EXIT_FAILURE);
#else
  std::exit(EXIT_FAILURE);
#endif
}

} // extern "C"
} // namespace

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

  ume_is_initialized = true;
}

/* First finalize the memory pool, then Kokkos */
void finalize() {
  if (ume_is_initialized) {
    GetMemPool().Pool().Finalize();
    Kokkos::finalize();
  }
  ume_is_finalized = true;
}

/* With error condition and backtrace, abort the job (if have MPI)
 * or process exit (if no MPI). This is an asynchronous operation:
 * all calling processes will write to standard out. */
extern "C" void error_stop(char const msg[]) {
  finalize();
  halt_with_backtrace(msg, true);
}

} // namespace Ume
