/*!
  \file Ume/Comm_Transport.hh
*/

#include "Ume/Comm_Transport.hh"
#include <cstdlib>
#include <iostream>

namespace Ume {
namespace Comm {

void Transport::abort(char const *const message) {
  std::cerr << "Transport::abort: " << message << std::endl;
  std::abort();
}

Dummy_Transport::Dummy_Transport() {
  std::cerr << "\n* WARNING * WARNING * WARNING * WARNING * WARNING "
               "* WARNING * WARNING *\n"
            << "\tA dummy transport mechanism was instantiated:\n"
            << "\tAll communications will silently fail!\n"
            << "* WARNING * WARNING * WARNING * WARNING * WARNING "
               "* WARNING * WARNING *\n"
            << std::endl;
}

} // namespace Comm
} // namespace Ume
