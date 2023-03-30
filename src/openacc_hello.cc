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
  \file openacc_hello.cc

  This is just a test that we can use the OpenACC programming model if enabled.
*/

#include <iostream>
#include <openacc.h>

int main() {

#pragma acc init

  auto dev = acc_get_device_type();
  int num = acc_get_device_num(dev);
  std::cout << "OpenACC Device " << num << " Info"
            << "\n";
  std::cout << "\tVendor: "
            << acc_get_property_string(num, dev, acc_property_vendor) << '\n';
  std::cout << "\t  Name: "
            << acc_get_property_string(num, dev, acc_property_name) << '\n';
  std::cout << "\tDriver: "
            << acc_get_property_string(num, dev, acc_property_driver) << '\n';

#pragma acc shutdown

  return 0;
}
