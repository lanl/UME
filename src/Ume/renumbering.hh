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
*/
#ifndef UME_RENUMBERING_HH
#define UME_RENUMBERING_HH 1

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/DS_Types.hh"

namespace Ume {

//! Mesh renumbering driver routine.
void renumber_mesh(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for points.
/*! Renumber points via advancing wavefront. */
void renumber_p_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for sides.
/*! Renumber sides via min/max point number. */
void renumber_s_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for zones.
/*! Renumber zones via min/max point number. */
void renumber_z_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for faces.
/*! Renumber faces via minimum side number. It only needs to be done
 * once because there is never more than one face attached to a side. */
void renumber_f_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for edges.
/*! Renumber edges via min/max point number. */
void renumber_e_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for corners.
/*! Renumber corners based on point order. */
void renumber_c_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for iotas.
/*! Renumber iotas based on point order. */
void renumber_a_maps(SOA_Idx::Mesh &mesh);

//! Get a new mesh ordering for wedges.
/*! Renumber wedges via min/max point number. */
void renumber_w_maps(SOA_Idx::Mesh &mesh);

//! Renumber entity X based on Y.
/*! Assumes that Y is already renumbered smoothly, so that ordering X
 * based on Y will produce a smooth ordering of X. */
void new_numbering(int const xl, auto const &x_type, int const yl,
                   DS_Types::INTV_T const &x_to_y_map,
                   int &x_max, DS_Types::INTV_T &x_to_xnew_map);

} // namespace Ume

#endif
