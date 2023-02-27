/*!
\file Ume/SOA_Idx_Edges.hh
*/

#ifndef UME_SOA_IDX_EDGES_HH
#define UME_SOA_IDX_EDGES_HH 1

#include "Ume/Entity_Field.hh"
#include "Ume/SOA_Entity.hh"

namespace Ume {
namespace SOA_Idx {

//! SoA representation of mesh edges (connects two points)
struct Edges : public Entity {
  explicit Edges(Mesh *mesh);
  void write(std::ostream &os) const override;
  void read(std::istream &is) override;
  void resize(int const local, int const total, int const ghost) override;
  bool operator==(Edges const &rhs) const;

  class DSE_ecoord : public Entity_Field<Edges> {
  public:
    explicit DSE_ecoord(Edges &e) : Entity_Field(Types::VEC3V, e) {}

  protected:
    bool init_() const override;
  };
};

} // namespace SOA_Idx
} // namespace Ume
#endif
