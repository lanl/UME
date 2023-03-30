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
