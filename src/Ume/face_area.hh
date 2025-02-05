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
   \file Ume/face_area.hh
*/
#ifndef UME_FACE_AREA_HH
#define UME_FACE_AREA_HH 1

#include "Ume/SOA_Idx_Mesh.hh"

namespace Ume {

//! Calculate the flat face area
/* This method computes the face areas for each face in the mesh by
 * accumulating side surface area vector magnitudes of internal sides
 * corresponding to internal or master faces in the MPI comm stencil,
 * making sure not to double count sides. */
void calc_face_area(SOA_Idx::Mesh &mesh, DS_Types::DBLV_T &face_area, int cali_record);

} // namespace Ume

#endif
