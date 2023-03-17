/*!
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
  volume.
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

//! Calculate the gradient of a zone-centered field at mesh points.
/*!
  This method computes the point-centered gradient of a zone-centered field by
  computing the piecewise-constant contour integral around the point control
  volume. This version uses a thread-safe connectivity.
*/
void gradzatp_invert(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &point_gradient);

//! Calculate the gradient of a zone-centered field at the zone centers.
/*!
  Calculate a zone-centered gradient from volume-weighted bounding-point
  gradients.  Returns both the zone-centered gradient and the point-centered
  gradient. This version uses a thread-safe connectivity.
 */
void gradzatz_invert(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &zone_gradient, DS_Types::VEC3V_T &point_gradient);

} // namespace Ume

#endif
