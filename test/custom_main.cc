/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

#include "Ume/process_mgmt.hh"
#include <catch2/catch_session.hpp>

/* Tests that need Kokkos and/or MPI require a custom main. */
int main(int argc, char *argv[]) {
  Ume::initialize(argc, argv);
  int result = Catch::Session().run(argc, argv);
  Ume::finalize();
  return result;
}
