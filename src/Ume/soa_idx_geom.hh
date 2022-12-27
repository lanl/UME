/*!
  \file soa_idx_grad.hh
*/

#ifndef SOA_IDX_GRAD_HH
#define SOA_IDX_GRAD_HH

#include "SOA_Idx_Mesh.hh"

namespace Ume {

namespace SOA_Idx {
void gradientZatZ(Mesh const &m, Entity::Subset const &s,
    std::vector<double> const &d, std::vector<PtCoord> grad);
}

} // namespace Ume

#endif
