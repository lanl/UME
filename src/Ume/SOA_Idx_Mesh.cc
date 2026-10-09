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
#include <ios>
#include <istream>
#include <ostream>

namespace Ume {
namespace SOA_Idx {

/* --------------------------------- Mesh -----------------------------------*/

Mesh::Mesh()
    : Mesh_Base(), corners{this}, edges{this}, faces{this}, points{this},
      sides{this}, zones{this}, iotas{this} {}

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
  write_bin(os, ivtag);
  write_bin(os, mype);
  write_bin(os, numpe);
  write_bin(os, geo);
  write_bin(os, dump_iotas);
  points.write(os);
  edges.write(os);
  faces.write(os);
  sides.write(os);
  corners.write(os);
  zones.write(os);
  if (dump_iotas)
    iotas.write(os);
}

void Mesh::read(std::istream &is) {
  read_bin(is, ivtag);

  // Handle the case when the only an old binary ume file is available:
  //
  // The ume binary was created before a heading existed in the umetxt
  // and the original umetxt is no longer available.
  //
  // Therefore, trying to read the version gives you mype since
  // no version information exists
  //
  // An alternative solution is to have a script to modify these
  // original binary ume files
  if (ivtag != UME_VERSION_1 && ivtag != UME_VERSION_2) {
    mype = ivtag;
    ivtag = UME_VERSION_1;
    dump_iotas = false;
    version_header = false;
    std::cout << "No Version Header" << std::endl;
  } else {
    read_bin(is, mype);
    version_header = true;
  }

  read_bin(is, numpe);
  read_bin(is, geo);

  if (version_header)
    read_bin(is, dump_iotas);

  /* In file order; the iotas are in the file only when dumped.  An entity
     whose tag is missing or wrong fails the stream and is left as it was, and
     the read stops at the first entity that leaves the stream failed. */
  Entity *const entities[] = {
      &points, &edges, &faces, &sides, &corners, &zones, &iotas};
  for (Entity *const e : entities) {
    if (e == &iotas && !dump_iotas)
      break;
    e->read(is);
    if (!is)
      return;
  }

  /* A scatter overwrites each copy from its buffer entry, which is only well
     defined if no copy is listed twice.  Checked here, once per mesh, rather
     than on every unpack. */
  for (Entity const *const e : entities) {
    if (!Comm::elements_unique(e->myCpys))
      is.setstate(std::ios::failbit);
  }
}

bool Mesh::operator==(Mesh const &rhs) const {
  return ivtag == rhs.ivtag && mype == rhs.mype && numpe == rhs.numpe &&
      geo == rhs.geo && dump_iotas == rhs.dump_iotas && points == rhs.points &&
      edges == rhs.edges && faces == rhs.faces && sides == rhs.sides &&
      corners == rhs.corners && zones == rhs.zones && iotas == rhs.iotas;
}

void Mesh::print_stats(std::ostream &os) const {
  os << "\tInput version: " << ivtag << '\n';
  os << "\tDecomposed Rank " << mype << " (" << mype + 1 << "/" << numpe << ')'
     << '\n';
  os << "\tPoint dimensions: " << ndims() << '\n';
  os << "\tCoordinate system: " << geo << '\n';
  os << "\tIotas dumped: " << std::boolalpha << dump_iotas << '\n';
  os << "\tPoints: " << points.local_size() << '\n';
  os << "\tZones: " << zones.local_size() << ' ' << zones.size() << '\n';
  os << "\tSides: " << sides.local_size() << ' ' << sides.size() << '\n';
  os << "\tEdges: " << edges.local_size() << '\n';
  os << "\tFaces: " << faces.local_size() << '\n';
  os << "\tCorners: " << corners.local_size() << ' ' << corners.size() << '\n';
  os << "\tIotas: " << iotas.local_size() << ' ' << iotas.size() << '\n';
}

} // namespace SOA_Idx
} // namespace Ume
