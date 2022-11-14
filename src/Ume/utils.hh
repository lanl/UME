/*!
\file utils.hh
*/

#include <istream>
#include <limits>

#ifndef UTILS_HH
#define UTILS_HH 1

namespace Ume {
inline void skip_line(std::istream &S) {
  S.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
} // namespace Ume
#endif
