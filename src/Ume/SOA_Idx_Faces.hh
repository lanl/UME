/*!
\file Ume/SOA_Idx_Faces.hh
*/

#ifndef UME_SOA_IDX_FACES_HH
#define UME_SOA_IDX_FACES_HH 1

#include "Ume/Entity_Field.hh"
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

  class DSE_fcoord : public Entity_Field<Faces> {
  public:
    explicit DSE_fcoord(Faces &f) : Entity_Field(Types::VEC3V, f) {}

  protected:
    bool init_() const override;
  };
};
} // namespace SOA_Idx
} // namespace Ume
#endif
