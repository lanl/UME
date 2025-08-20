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
  \file Ume/SOA_Idx_Mesh.hh
*/

#ifndef UME_SOA_IDX_MESH_HH
#define UME_SOA_IDX_MESH_HH 1

/*! Input version tags. These document breaking changes in UME input
 * decks. All inputs of a particular version are valid up to the next
 * version number. */

/*! 1.0.0 release tag. */
#define UME_VERSION_1 20230330
/*! The latest input version tag. Inputs include iota information. */
#define UME_VERSION_2 20250722

#include "Ume/Mesh_Base.hh"
#include "Ume/SOA_Entity.hh"
#include "Ume/SOA_Idx_Corners.hh"
#include "Ume/SOA_Idx_Edges.hh"
#include "Ume/SOA_Idx_Faces.hh"
#include "Ume/SOA_Idx_Points.hh"
#include "Ume/SOA_Idx_Sides.hh"
#include "Ume/SOA_Idx_Zones.hh"
#include "Ume/SOA_Idx_Iotas.hh"
#include <iosfwd>

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

using Types = Ume::DS_Types::Types;

//! An Struct-Of-Arrays Mesh
struct Mesh : public Mesh_Base {
  enum Geometry_Type { CARTESIAN, CYLINDRICAL, SPHERICAL };
  int ivtag;
  bool version_header;
  int mype;
  int numpe;
  Geometry_Type geo;
  bool dump_iotas;
  Corners corners;
  Edges edges;
  Faces faces;
  Points points;
  Sides sides;
  Zones zones;
  Iotas iotas;
  Mesh();
  void write(std::ostream &os) const;
  void read(std::istream &is);
  constexpr size_t ndims() const { return 3; }
  bool operator==(Mesh const &rhs) const;
  void print_stats(std::ostream &os) const;
};

} // namespace SOA_Idx
} // namespace Ume

#endif
