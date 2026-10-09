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
  \file error_stop_probe.cc

  Initializes Ume, claims 512 KiB from the memory pool and then 1 MiB more:

      MEMORY_POOL_SIZE_MB=<n> error_stop_probe

  With a 1 MB pool the second claim does not fit, and Claim goes through
  Ume::error_stop with "Memory pool cannot claim memory.", leaving the first
  claim outstanding.  With a pool of 2 MB or more both claims fit; the probe
  releases them, finalizes and exits 0.  MPI is not initialized, in any build:
  error_stop has to report before it reaches halt() either way.
*/

#include "Ume/memory.hh"
#include "Ume/process_mgmt.hh"
#include <cstddef>
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[]) {
  Ume::initialize(argc, argv);

  constexpr std::size_t KiB = 1024;
  auto &pool = GetMemPool().Pool();
  void *const half = pool.Claim(512 * KiB);
  void *const rest = pool.Claim(1024 * KiB);

  std::printf("error_stop_probe: both claims fit\n");
  pool.Release(rest);
  pool.Release(half);
  Ume::finalize();
  return EXIT_SUCCESS;
}
