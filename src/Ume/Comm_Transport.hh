/*!
\file Ume/Comm_Transport.hh
*/

#ifndef COMM_TRANSPORT_HH
#define COMM_TRANSPORT_HH 1

namespace Ume {
namespace Comm {

class Transport {
public:
  enum class CommOp { SUM };

public:
  virtual int stop() { return -1; };
  virtual ~Transport() = default;
};

} // namespace Comm
} // namespace Ume

#endif
