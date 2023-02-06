/*
   \file Ume/gradient.hh
*/
#ifndef UME_GRADIENT_HH
#define UME_GRADIENT_HH 1

#include "Ume/SOA_Idx_Mesh.hh"

namespace Ume {

//! Calculate the gradient of a zone-centered field at mesh points.
/*!
  This method computes the point-centered gradient of a zone-centered field by
  computing the piecewise-constant contour integral around the point control
  volume.  It also returns the point control volumes, which are sometimes needed
  by the caller.
*/
void gradzatp(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &point_gradient);

//! Calculate the gradient of a zone-centered field at the zone centers.
/*!
  Calculate a zone-centered gradient from volume-weighted bounding-point
  gradients.  Returns both the zone-centered gradient and the point-centered
  gradient.
 */
void gradzatz(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &zone_gradient, DS_Types::VEC3V_T &point_gradient);

} // namespace Ume

#endif
