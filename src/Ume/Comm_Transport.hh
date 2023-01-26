/*!
\file Ume/Comm_Transport.hh
*/

#ifndef COMM_TRANSPORT_HH
#define COMM_TRANSPORT_HH 1

#include "Ume/Comm_Buffers.hh"

namespace Ume {
namespace Comm {

class Transport {
public:
  virtual void exchange(Buffers<DS_Types::INTV_T> const & /*sends*/,
      Buffers<DS_Types::INTV_T> & /*recvs*/) {}
  virtual void exchange(Buffers<DS_Types::DBLV_T> const & /*sends*/,
      Buffers<DS_Types::DBLV_T> & /*recvs*/) {}
  virtual void exchange(Buffers<DS_Types::VEC3V_T> const & /*sends*/,
      Buffers<DS_Types::VEC3V_T> & /*recvs*/) {}
  virtual int id() const { return -1; }
  virtual int stop() { return -1; }
  virtual ~Transport() = default;
};

} // namespace Comm
} // namespace Ume

#endif
