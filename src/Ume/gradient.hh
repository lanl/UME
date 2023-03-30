/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

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
