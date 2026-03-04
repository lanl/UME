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
    const char *envPoolSizeMB = std::getenv("LOKKOS_POOL_SIZE_MB");
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

} // namespace Ume
