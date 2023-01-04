/*
\file SOA_Idx_Faces.hh
*/

#ifndef SOA_IDX_FACES_HH
#define SOA_IDX_FACES_HH 1

#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

//! SoA representation of mesh faces (separates zones)
struct Faces : public Entity {
  explicit Faces(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Faces const &rhs) const;

  class DSE_fcoord : public DS_Entry {
  public:
    explicit DSE_fcoord(Faces &f) : DS_Entry(Types::VEC3V), faces_{f} {}

  protected:
    void init_() const override;

  private:
    Faces const &faces_;
  };
};
} // namespace SOA_Idx
} // namespace Ume
#endif
