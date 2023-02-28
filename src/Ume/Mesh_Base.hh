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
