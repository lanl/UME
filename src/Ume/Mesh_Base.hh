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
  \file Ume/Mesh_Base.hh
*/
#ifndef UME_MESH_BASE_HH
#define UME_MESH_BASE_HH 1

#include "Ume/Comm_Transport.hh"
#include "Ume/Datastore.hh"

namespace Ume {

//! Common to any mesh implementation
/*! This class exists mostly to solve chicken-and-egg problems with Datastore
    and Transport */
struct Mesh_Base {
  Mesh_Base() : ds{Datastore::create_root()} {}
  Datastore::dsptr ds;
  Ume::Comm::Transport *comm = nullptr;
};

} // namespace Ume

#endif
