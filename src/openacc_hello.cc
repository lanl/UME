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
