#ifndef PT_HH
#define PT_HH 1

namespace Ume {

template <unsigned D> struct Pt {
public:
  constexpr double operator[](int const idx) const { return comp[idx]; }
  constexpr double &operator[](int const idx) { return comp[idx]; }

private:
  double comp[D];
};

} // namespace Ume
#endif
