/*
   \file Ume/gradient.hh
*/
#ifndef UME_GRADIENT_HH
#define UME_GRADIENT_HH 1

#include "Ume/SOA_Idx_Mesh.hh"

namespace Ume {

//!  Corner-based control volume gradient method.
/*!
  This method computes the point-centered gradient of a zone-centered field by
  computing the piecewise-constant contour integral around the point control
  volume.  It also returns the point control volumes which are sometimes needed
  by the caller.
*/

void gradzatp(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zfield,
    DS_Types::VEC3V_T &pgrad, DS_Types::DBLV_T &pvol);
} // namespace Ume

#endif
