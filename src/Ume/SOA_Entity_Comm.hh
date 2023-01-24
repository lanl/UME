#include <cassert>

namespace Ume {
namespace SOA_Idx {

template <typename FT> void Entity::gather(Comm::Op const /*op*/, FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> recvBufs(mySrcs);
  Comm::Buffers<FT> sendBufs(myCpys);
  sendBufs.pack(field);
  // exchange(sendBufs, recvBufs)
  recvBufs.unpack(field);
}

template <typename FT> void Entity::scatter(FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> recvBufs(myCpys);
  Comm::Buffers<FT> sendBufs(mySrcs);
  sendBufs.pack(field);
  // exchange(sendBufs, recvBufs)
  recvBufs.unpack(field);
}

template <typename FT> void Entity::gathscat(Comm::Op const /*op*/, FT &field) {
  assert(static_cast<int>(field.size()) == size());
  Comm::Buffers<FT> srcBufs(mySrcs);
  Comm::Buffers<FT> cpyBufs(myCpys);
  cpyBufs.pack(field);
  // exchange(cpyBufs, srcBufs)
  // combine
  // exhange(srcBufs, cpyBufs)
  cpyBufs.unpack(field);
}

} // namespace SOA_Idx
} // namespace Ume
