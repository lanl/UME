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
   \file Ume/renumbering.hh
 * Implementation of a renumbering package known as "Wavefront min/max
 * renumbering" which optimizes memory data locality by the following
 * techniques: wavefront renumbering for mesh points, minimum/maximum
 * renumbering for all other mesh entities. The data for each processor
 * is renumbered separately.
 *
 * The call chains invoked by the renumbering node are large and
 * largely opaque, driven by the AJAX broadcast mechanism which we
 * emulate here. In this way, messages are broadcast to the mesh entity
 * to drive renumbering. The message propagates down the subtree of the
 * database rooted at the mesh handle such that each mesh entity may
 * respond to the given message via dynamic dispatch. There is no way
 * to know how many, if any, mesh entities needing renumbering will
 * exist prior to a simulation from which the UME mesh inputs are
 * derived. It is possible for the renumbering scheme to be a no-op.
 *
 * NOTE: broadcast dispatch is sequential and follows a canonical order
 * that may not be done in parallel.
*/
#ifndef UME_RENUMBERING_HH
#define UME_RENUMBERING_HH 1

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/DS_Types.hh"
#include <cassert>

namespace Ume {

//! Mesh renumbering driver routine.
void renumber_mesh(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for points.
/*! Renumber points via advancing wavefront. */
void renumber_p(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for sides.
/*! Renumber sides via min/max point number. */
void renumber_s(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for zones.
/*! Renumber zones via min/max point number. */
void renumber_z(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for faces.
/*! Renumber faces via minimum side number. It only needs to be done
 * once because there is never more than one face attached to a side. */
void renumber_f(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for edges.
/*! Renumber edges via min/max point number. */
void renumber_e(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for corners.
/*! Renumber corners based on point order. */
void renumber_c(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for iotas.
/*! Renumber iotas based on point order. */
void renumber_a(SOA_Idx::Mesh &mesh);

//! Renumber entity X based on Y.
/*! Assumes that Y is already renumbered smoothly, so that ordering X
 * based on Y will produce a smooth ordering of X. */
template <typename MeshEntity1, typename MeshEntity2>
void new_numbering(MeshEntity1 const &x, MeshEntity2 const &y,
                    DS_Types::INTV_T const &x_to_y_map,
                    int &x_max, DS_Types::INTV_T &x_to_xnew_map) {
  static_assert(std::is_base_of<SOA_Idx::Entity, MeshEntity1>::value);
  static_assert(std::is_base_of<SOA_Idx::Entity, MeshEntity2>::value);
  assert(x.size() == static_cast<int>(x_to_y_map.size()));
  assert(x.size() == static_cast<int>(x_to_xnew_map.size()));

  int const yl1 = y.local_size() + 1;
  DS_Types::INTV_T storage_locations(yl1, 0);

  auto const &x_type = x.mask;
  /* Count x attached to each y. */
  for (int x_idx : x.local_indices()) {
    if (x_type[x_idx] == 0)
      continue;

    int const y_idx = x_to_y_map[x_idx];
    storage_locations[y_idx + 1] += 1;
  }

  /* Sum storage locations. */
  for (int y_idx : std::ranges::iota_view{1, yl1 - 1}) {
    storage_locations[y_idx] += storage_locations[y_idx - 1];
  }

  /* Set new numbers. */
  for (int x_idx : x.local_indices()) {
    if (x_type[x_idx] == 0)
      continue;

    int const y_idx = x_to_y_map[x_idx];
    x_to_xnew_map[x_idx] = storage_locations[y_idx];
    x_max = std::max(x_max, storage_locations[y_idx]);
  }
}

} // namespace Ume

#endif
