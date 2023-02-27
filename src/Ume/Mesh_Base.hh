/*!
  \file Ume/Mesh_Base.hh
*/
#ifndef UME_MESH_BASE_HH
#define UME_MESH_BASE_HH 1

#include "Ume/Comm_Transport.hh"
#include "Ume/Datastore.hh"
#include <memory>

namespace Ume {

struct Mesh_Base {
  Mesh_Base() : ds{Datastore::create_root()} {}
  Datastore::dsptr ds;
  Ume::Comm::Transport *comm = nullptr;
};

} // namespace Ume

#endif
