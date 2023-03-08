/*!
   \file Ume/gradient.hh
*/
#ifndef UME_GRADIENT_HH
#define UME_GRADIENT_HH 1

/*
** Scoria Includes
*/
#ifdef USE_SCORIA
extern "C" {
#include "client.h"
#include "config.h"

#include "client_cleanup.h"
#include "client_init.h"
#include "client_memory.h"
#include "client_wait_requests.h"

#include "shm_malloc.h"
}
#endif /* USE_SCORIA */

/*
** Ume Includes
*/
#include "Ume/SOA_Idx_Mesh.hh"
#include "shm_allocator.hh"

namespace Ume {

//! Calculate the gradient of a zone-centered field at mesh points.
/*!
  This method computes the point-centered gradient of a zone-centered field by
  computing the piecewise-constant contour integral around the point control
  volume.  It also returns the point control volumes, which are sometimes needed
  by the caller.
*/

#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatp(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DS_Types::DBLV_T const &zone_field, DS_Types::VEC3V_T &point_gradient);
#else
void gradzatp(Ume::SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &point_gradient);
#endif

//! Calculate the gradient of a zone-centered field at the zone centers.
/*!
  Calculate a zone-centered gradient from volume-weighted bounding-point
  gradients.  Returns both the zone-centered gradient and the point-centered
  gradient.
 */
#if defined(USE_SCORIA) && defined(USE_CLIENT)
void gradzatz(struct client *client, Ume::SOA_Idx::Mesh &mesh,
    DS_Types::DBLV_T const &zone_field, DS_Types::VEC3V_T &zone_gradient,
    DS_Types::VEC3V_T &point_gradient);
#else
void gradzatz(Ume::SOA_Idx::Mesh &mesh, DS_Types::DBLV_T const &zone_field,
    DS_Types::VEC3V_T &zone_gradient, DS_Types::VEC3V_T &point_gradient);
#endif
} // namespace Ume

#endif
