/*!
  \file Ume/Comm_Transport.hh
*/

#include "Comm_Transport.hh"
#include <iostream>

namespace Ume {
namespace Comm {

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
