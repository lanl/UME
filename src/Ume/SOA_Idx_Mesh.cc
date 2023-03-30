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
  \file Ume/SOA_Idx_Mesh.cc
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/soa_idx_helpers.hh"
#include <istream>
#include <ostream>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Mesh -----------------------------------*/

Mesh::Mesh()
    : Mesh_Base(), corners{this}, edges{this}, faces{this}, points{this},
      sides{this}, zones{this} {}

std::ostream &operator<<(std::ostream &os, Mesh::Geometry_Type const &geo) {
  switch (geo) {
  case Mesh::CARTESIAN:
    os << "Cartesian";
    break;
  case Mesh::CYLINDRICAL:
    os << "Cylindrical";
    break;
  case Mesh::SPHERICAL:
    os << "Spherical";
    break;
  };
  return os;
}

void Mesh::write(std::ostream &os) const {
  write_bin(os, mype);
  write_bin(os, numpe);
  write_bin(os, geo);
  points.write(os);
  edges.write(os);
  faces.write(os);
  sides.write(os);
  corners.write(os);
  zones.write(os);
}

void Mesh::read(std::istream &is) {
  read_bin(is, mype);
  read_bin(is, numpe);
  read_bin(is, geo);
  points.read(is);
  edges.read(is);
  faces.read(is);
  sides.read(is);
  corners.read(is);
  zones.read(is);
}

bool Mesh::operator==(Mesh const &rhs) const {
  return mype == rhs.mype && numpe == rhs.numpe && geo == rhs.geo &&
      points == rhs.points && edges == rhs.edges && sides == rhs.sides &&
      corners == rhs.corners && zones == rhs.zones;
}

void Mesh::print_stats(std::ostream &os) const {
  os << "\tDecomposed Rank " << mype << " (" << mype + 1 << "/" << numpe << ')'
     << '\n';
  os << "\tPoint dimensions: " << ndims() << '\n';
  os << "\tCoordinate system: " << geo << '\n';
  os << "\tPoints: " << points.local_size() << '\n';
  os << "\tZones: " << zones.local_size() << ' ' << zones.size() << '\n';
  os << "\tSides: " << sides.local_size() << ' ' << sides.size() << '\n';
  os << "\tEdges: " << edges.local_size() << '\n';
  os << "\tFaces: " << faces.local_size() << '\n';
  os << "\tCorners: " << corners.local_size() << ' ' << corners.size() << '\n';
}

} // namespace SOA_Idx
} // namespace Ume
