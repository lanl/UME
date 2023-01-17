#include <cassert>

namespace Ume {
namespace SOA_Idx {

template <typename FT> void Entity::gathscat(Comm::Op const op, FT &field) {
  assert(static_cast<int>(field.size()) == size());
  mesh_->comm->move(op);
}

} // namespace SOA_Idx
} // namespace Ume
