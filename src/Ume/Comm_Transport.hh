/*!
\file Ume/Comm_Transport.hh
*/

#ifndef COMM_TRANSPORT_HH
#define COMM_TRANSPORT_HH 1

namespace Ume {
namespace Comm {

enum class Op { SUM };

class Transport {
public:
  virtual void move(Op /*op */) {}
  virtual int stop() { return -1; };
  virtual ~Transport() = default;
};

} // namespace Comm
} // namespace Ume

#endif
