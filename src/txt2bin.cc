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
  \file txt2bin.cc

  This reads a file in the LAP ASCII UmeDump format (see
  `$OPUS/DELFI/Output/UmeDump`), and creates a binary file for use with
  `Ume/SOA_Idx_Mesh.hh`.  Note that this only reads 3-D meshes.
*/

#include "Ume/SOA_Idx_Mesh.hh"
#include "Ume/Timer.hh"
#include "Ume/utils.hh"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

using namespace Ume::SOA_Idx;
using Ume::skip_line;

int read(std::istream &is, Mesh &m);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: txt2bin <infile> <outfile>" << std::endl;
    return 1;
  }

  double txt_read_time;
  Mesh m;
  {
    int retval;
    std::cout << "Reading text file \"" << argv[1] << "\"" << std::endl;
    std::ifstream is(argv[1]);
    if (!is) {
      std::cerr << "Couldn't open \"" << argv[1] << "\" for reading"
                << std::endl;
      return 2;
    }
    Ume::Timer timer;
    timer.start();
    retval = read(is, m);
    timer.stop();
    is.close();
    if (retval) {
      std::cerr << "exiting due to read errors" << std::endl;
      return 3;
    } else {
      std::cout << "Text read took " << timer << "\n";
      txt_read_time = timer.seconds();
    }
  }

  {
    std::cout << "Writing binary file \"" << argv[2] << "\"" << std::endl;
    std::ofstream os(argv[2]);
    if (!os) {
      std::cerr << "Couldn't open \"" << argv[2] << "\" for writing"
                << std::endl;
      return 4;
    }
    Ume::Timer timer;
    timer.start();
    m.write(os);
    timer.stop();
    os.close();
    std::cout << "Binary write took " << timer << "\n";
  }

  {
    Mesh m2;
    std::cout << "Reading binary file \"" << argv[2] << "\" for verification"
              << std::endl;
    std::ifstream is(argv[2]);
    Ume::Timer timer;
    timer.start();
    m2.read(is);
    timer.stop();
    std::cout << "Binary read took " << timer << " ("
              << txt_read_time / timer.seconds() << "x speedup)\n";
    is.close();
    if (!(m == m2)) {
      std::cerr << "Error: write/read test failed, meshes not equivalent."
                << std::endl;
      return 5;
    } else {
      std::cout << "Copy verified" << std::endl;
      std::cout << "\nMesh Stats\n-------------------------------\n";
      m2.print_stats(std::cout);
      std::cout << std::endl;
    }
  }

  return 0;
}

int read_tag(std::istream &is, char const *const expect) {
  char tagname[25];
  is.get(tagname, 23);
  std::string ts(tagname);

  while (ts.back() == ':' || ts.back() == ' ')
    ts.pop_back();
  if (ts != std::string(expect)) {
    std::cerr << "Expecting tag \"" << expect << "\", got \"" << ts << "\""
              << std::endl;
    exit(1);
  }
  int val{-1};
  is >> val;
  if (!is) {
    std::cerr << "Didn't find an integer after tag \"" << ts << "\""
              << std::endl;
    exit(1);
  }
  is >> std::ws;
  return val;
}

int read_vtag(std::istream &is, char const *const expect) {
  /* If "Input version" tag is absent, default to old input version. */
  char const c1 = static_cast<char>(is.peek());
  if (c1 != 'I')
    return UME_VERSION_1;

  char tagname[25];
  is.get(tagname, 23);
  std::string ts(tagname);

  while (ts.back() == ':' || ts.back() == ' ')
    ts.pop_back();

  if (ts != std::string(expect)) {
    std::cerr << "Expecting tag \"" << expect << "\", got \"" << ts << "\""
              << std::endl;
    exit(EXIT_FAILURE);
  }

  int val = -1;
  is >> val;
  if (!is) {
    std::cerr << "Didn't find an integer after tag \"" << ts << "\""
              << std::endl;
    exit(EXIT_FAILURE);
  }
  is >> std::ws;
  return val;
}

bool read_bool_tag(std::istream &is, char const *const expect) {
  char tagname[25];
  is.get(tagname, 23);
  std::string ts(tagname);

  while (ts.back() == ':' || ts.back() == ' ')
    ts.pop_back();

  if (ts != std::string(expect)) {
    std::cerr << "Expecting tag \"" << expect << "\", got \"" << ts << "\""
              << std::endl;
    exit(EXIT_FAILURE);
  }

  bool val = false;
  is >> std::boolalpha >> val;
  if (!is) {
    std::cerr << "Didn't find an integer after tag \"" << ts << "\""
              << std::endl;
    exit(EXIT_FAILURE);
  }
  is >> std::ws;
  return val;
}

std::string read_tag_str(std::istream &is, char const *const expect) {
  char tagname[25];
  is.get(tagname, 23);
  std::string ts(tagname);

  while (ts.back() == ':' || ts.back() == ' ')
    ts.pop_back();
  if (ts != std::string(expect)) {
    std::cerr << "Expecting tag \"" << expect << "\", got \"" << ts << "\""
              << std::endl;
    exit(1);
  }
  std::string val;
  if (!std::getline(is, val)) {
    std::cerr << "Error reading string after tag \"" << ts << "\"" << std::endl;
    exit(1);
  }
  is >> std::ws;
  return Ume::trim(val);
}

bool expect_line(std::istream &is, char const *const expect) {
  const size_t bufsize{256};
  char buf[bufsize];
  buf[0] = '\0';
  is.getline(buf, bufsize);
  if (!is || std::strcmp(buf, expect)) {
    std::cerr << "Expecting line \"" << expect << "\", got \"" << buf << '\n';
    exit(1);
  }
  is >> std::ws;
  return true;
}

bool skip_to_line(std::istream &is, char const *const expect) {
  std::string buf;
  std::string const search{expect};

  while (is && buf != search) {
    std::getline(is, buf);
  }
  if (!is)
    return false;
  return true;
}

void read_points(std::istream &is, Points &pts, const int kkpl, const int kkpll,
    const int kkpgl, const int ndims) {

  pts.resize(kkpl, kkpll, kkpgl);
  auto &coords = pts.ds().access_vec3v("pcoord");

  int idx;
  for (int i = 0; i < kkpll; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Point " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> pts.mask[i] >> pts.comm_type[i] >> coords[i][0];
    for (int d = 1; d < ndims; ++d) {
      is >> coords[i][d];
    }
    is >> std::ws;
  }

  expect_line(is, "Ghost Points");
  skip_line(is);
  for (int i = 0; i < kkpgl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost point " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> pts.cpy_idx[i] >> pts.ghost_mask[i] >> pts.src_idx[i] >>
        pts.src_pe[i] >> std::ws;
    --pts.cpy_idx[i];
    --pts.src_idx[i];
  }
}

void read_zones(std::istream &is, Zones &zones, const int kkzl, const int kkzll,
    const int kkzgl) {
  int idx;
  zones.resize(kkzl, kkzll, kkzgl);
  for (int i = 0; i < kkzll; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Zone " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> zones.mask[i] >> zones.comm_type[i] >> std::ws;
  }

  expect_line(is, "Ghost Zones");
  skip_line(is);
  for (int i = 0; i < kkzgl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost zone " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> zones.cpy_idx[i] >> zones.ghost_mask[i] >> zones.src_idx[i] >>
        zones.src_pe[i] >> std::ws;
    --zones.cpy_idx[i];
    --zones.src_idx[i];
  }
}

/***********************************************************************
 * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT
 ***********************************************************************
 Remember that all of the indices in the text file are in Fortran base-1.
 Subtract one when reading them into C/C++.  mask is a flag, not an
 index, so don't touch that.
 ***********************************************************************
 * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT * IMPORTANT
 ***********************************************************************
*/

void read_sides(std::istream &is, Sides &sides, const int kksl, const int kksll,
    const int kksgl) {
  int idx;
  sides.resize(kksl, kksll, kksgl);
  auto &z = sides.ds().access_intv("m:s>z");
  auto &p1 = sides.ds().access_intv("m:s>p1");
  auto &p2 = sides.ds().access_intv("m:s>p2");
  auto &e = sides.ds().access_intv("m:s>e");
  auto &f = sides.ds().access_intv("m:s>f");
  auto &c1 = sides.ds().access_intv("m:s>c1");
  auto &c2 = sides.ds().access_intv("m:s>c2");
  auto &s2 = sides.ds().access_intv("m:s>s2");
  auto &s3 = sides.ds().access_intv("m:s>s3");
  auto &s4 = sides.ds().access_intv("m:s>s4");
  auto &s5 = sides.ds().access_intv("m:s>s5");

  for (int i = 0; i < kksll; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Side " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> sides.mask[i] >> sides.comm_type[i];
    is >> z[i] >> p1[i] >> p2[i] >> e[i] >> f[i] >> c1[i] >> c2[i] >> s2[i] >>
        s3[i] >> s4[i] >> s5[i] >> std::ws;
    --z[i];
    --p1[i];
    --p2[i];
    --e[i];
    --f[i];
    --c1[i];
    --c2[i];
    --s2[i];
    --s3[i];
    --s4[i];
    --s5[i];
  }
  expect_line(is, "Ghost Sides");
  skip_line(is);
  for (int i = 0; i < kksgl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost side " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> sides.cpy_idx[i] >> sides.ghost_mask[i] >> sides.src_idx[i] >>
        sides.src_pe[i] >> std::ws;
    --sides.cpy_idx[i];
    --sides.src_idx[i];
  }
}

void read_iotas(std::istream &is, Iotas &iotas, const int kkal, const int kkall,
    const int kkagl) {
  int idx;
  iotas.resize(kkal, kkall, kkagl);
  auto &z = iotas.ds().access_intv("m:a>z");
  auto &f = iotas.ds().access_intv("m:a>f");
  auto &p = iotas.ds().access_intv("m:a>p");
  auto &e = iotas.ds().access_intv("m:a>e");
  auto &s = iotas.ds().access_intv("m:a>s");

  for (int i = 0; i < kkall; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Iota " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> iotas.mask[i] >> iotas.comm_type[i];
    is >> z[i] >> f[i] >> p[i] >> e[i] >> s[i] >> std::ws;
    --z[i];
    --f[i];
    --p[i];
    --e[i];
    --s[i];
  }
  expect_line(is, "Ghost Iotas");
  skip_line(is);
  for (int i = 0; i < kkagl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost iota " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> iotas.cpy_idx[i] >> iotas.ghost_mask[i] >> iotas.src_idx[i] >>
        iotas.src_pe[i] >> std::ws;
    --iotas.cpy_idx[i];
    --iotas.src_idx[i];
  }
}

void read_edges(std::istream &is, Edges &edges, const int kkel, const int kkell,
    const int kkegl) {
  int idx;
  edges.resize(kkel, kkell, kkegl);
  auto &p1 = edges.ds().access_intv("m:e>p1");
  auto &p2 = edges.ds().access_intv("m:e>p2");
  for (int i = 0; i < kkell; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Edge " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> edges.mask[i] >> edges.comm_type[i] >> p1[i] >> p2[i] >> std::ws;
    --p1[i];
    --p2[i];
  }
  expect_line(is, "Ghost Edges");
  skip_line(is);
  for (int i = 0; i < kkegl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost edge " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> edges.cpy_idx[i] >> edges.ghost_mask[i] >> edges.src_idx[i] >>
        edges.src_pe[i] >> std::ws;
    --edges.cpy_idx[i];
    --edges.src_idx[i];
  }
}

void read_faces(std::istream &is, Faces &faces, const int kkfl, const int kkfll,
    const int kkfgl) {
  int idx;
  faces.resize(kkfl, kkfll, kkfgl);
  auto &z1 = faces.ds().access_intv("m:f>z1");
  auto &z2 = faces.ds().access_intv("m:f>z2");
  for (int i = 0; i < kkfll; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Face " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> faces.mask[i] >> faces.comm_type[i] >> z1[i] >> z2[i] >> std::ws;
    --z1[i];
    --z2[i];
  }
  expect_line(is, "Ghost Faces");
  skip_line(is);
  for (int i = 0; i < kkfgl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost face " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> faces.cpy_idx[i] >> faces.ghost_mask[i] >> faces.src_idx[i] >>
        faces.src_pe[i] >> std::ws;
    --faces.cpy_idx[i];
    --faces.src_idx[i];
  }
}

void read_corners(std::istream &is, Corners &corners, const int kkcl,
    const int kkcll, const int kkcgl) {
  int idx;
  corners.resize(kkcl, kkcll, kkcgl);
  auto &p = corners.ds().access_intv("m:c>p");
  auto &z = corners.ds().access_intv("m:c>z");
  for (int i = 0; i < kkcll; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Face " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> corners.mask[i] >> corners.comm_type[i] >> p[i] >> z[i] >> std::ws;
    --p[i];
    --z[i];
  }
  expect_line(is, "Ghost Corners");
  skip_line(is);
  for (int i = 0; i < kkcgl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Ghost corner " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> corners.cpy_idx[i] >> corners.ghost_mask[i] >> corners.src_idx[i] >>
        corners.src_pe[i] >> std::ws;
    --corners.cpy_idx[i];
    --corners.src_idx[i];
  }
}

void read_mpi(std::istream &is, Entity &e) {
  expect_line(is, "Recv From");
  int numpes = read_tag(is, "num PEs");
  int total_elem = read_tag(is, "total elem");
  e.myCpys.resize(numpes);
  int totalCount = 0;
  for (int i = 0; i < numpes; ++i) {
    e.myCpys[i].pe = read_tag(is, "rmt PE");
    int const elem_count = read_tag(is, "num elem");
    e.myCpys[i].elements.resize(elem_count);
    for (int j = 0; j < elem_count; ++j) {
      is >> e.myCpys[i].elements[j];
      if (!is) {
        std::cerr << "read_mpi: input error on RecvFrom " << i << ' ' << j
                  << std::endl;
        exit(1);
      }
      --e.myCpys[i].elements[j];
    }
    is >> std::ws;
    totalCount += elem_count;
  }
  if (totalCount != total_elem) {
    std::cerr << "read_mpi error myCpys" << std::endl;
    exit(1);
  }
  expect_line(is, "Send To");
  numpes = read_tag(is, "num PEs");
  total_elem = read_tag(is, "total elem");
  e.mySrcs.resize(numpes);
  totalCount = 0;
  for (int i = 0; i < numpes; ++i) {
    e.mySrcs[i].pe = read_tag(is, "rmt PE");
    int const elem_count = read_tag(is, "num elem");
    e.mySrcs[i].elements.resize(elem_count);
    for (int j = 0; j < elem_count; ++j) {
      is >> e.mySrcs[i].elements[j];
      if (!is) {
        std::cerr << "read_mpi: input error on SendTo " << i << ' ' << j
                  << std::endl;
        exit(1);
      }
      --e.mySrcs[i].elements[j];
    }
    is >> std::ws;
    totalCount += elem_count;
  }
  if (totalCount != total_elem) {
    std::cerr << "read_mpi error mySrcs" << std::endl;
    exit(1);
  }
}

int read(std::istream &is, Mesh &m) {
  m.ivtag = read_vtag(is, "Input version");
  m.numpe = read_tag(is, "Total ranks");
  m.mype = read_tag(is, "This rank");
  int ndims = read_tag(is, "Num dims");
  if (ndims != 3) {
    std::cerr << "Error: Ume only works on 3-D meshes, and this input is "
              << ndims << "-D." << std::endl;
    return 1;
  }
  int igeo = read_tag(is, "Geometry type");
  switch (igeo) {
  case 0:
    m.geo = Mesh::CARTESIAN;
    break;
  case 1:
    m.geo = Mesh::CYLINDRICAL;
    break;
  case 2:
    m.geo = Mesh::SPHERICAL;
    break;
  default:
    std::cerr << "Unknown igeo flag = " << igeo << '\n';
    return 2;
  }
  if (m.ivtag >= UME_VERSION_2) {
    m.dump_iotas = read_bool_tag(is, "Iotas dumped");
  } else {
    m.dump_iotas = false;
  }

  /*
    The "Number of <entity>" tags represent kkXl. The "Dimension:
    <entity>" tags represent kkXll. See
    $OPUS/DELFI/ParallelLib/DELFI_Parallel_Library_Guide.html for the
    difference between master/slave and ghost/real paradigms, and how
    they are stored in (1:kkXl) and (kkXl+1:kkXll).  Also see
    $OPUS/DELFI/Mesh/MeshBase.dic for the meanings of kXtyp, which
    also differentiates between on/off processor information.
  */
  const int kkpll = read_tag(is, "Point total");
  const int kkpl = read_tag(is, "Point local");
  const int kkpgl = read_tag(is, "Point ghost");
  const int kkzll = read_tag(is, "Zone total");
  const int kkzl = read_tag(is, "Zone local");
  const int kkzgl = read_tag(is, "Zone ghost");
  const int kksll = read_tag(is, "Side total");
  const int kksl = read_tag(is, "Side local");
  const int kksgl = read_tag(is, "Side ghost");
  const int kkell = read_tag(is, "Edge total");
  const int kkel = read_tag(is, "Edge local");
  const int kkegl = read_tag(is, "Edge ghost");
  const int kkfll = read_tag(is, "Face total");
  const int kkfl = read_tag(is, "Face local");
  const int kkfgl = read_tag(is, "Face ghost");
  const int kkcll = read_tag(is, "Corner total");
  const int kkcl = read_tag(is, "Corner local");
  const int kkcgl = read_tag(is, "Corner ghost");
  int kkall = 0;
  int kkal = 0;
  int kkagl = 0;
  if (m.ivtag >= UME_VERSION_2) {
    kkall = read_tag(is, "Iota total");
    kkal = read_tag(is, "Iota local");
    kkagl = read_tag(is, "Iota ghost");
  }

  expect_line(is, "Points");
  skip_line(is);
  read_points(is, m.points, kkpl, kkpll, kkpgl, ndims);

  skip_to_line(is, "Zones");
  skip_line(is);
  read_zones(is, m.zones, kkzl, kkzll, kkzgl);

  skip_to_line(is, "Sides");
  skip_line(is);
  read_sides(is, m.sides, kksl, kksll, kksgl);

  skip_to_line(is, "Edges");
  skip_line(is);
  read_edges(is, m.edges, kkel, kkell, kkegl);

  skip_to_line(is, "Faces");
  skip_line(is);
  read_faces(is, m.faces, kkfl, kkfll, kkfgl);

  skip_to_line(is, "Corners");
  skip_line(is);
  read_corners(is, m.corners, kkcl, kkcll, kkcgl);

  if (m.dump_iotas) {
    skip_to_line(is, "Iotas");
    skip_line(is);
    read_iotas(is, m.iotas, kkal, kkall, kkagl);
  }

  const int has_mpi = read_tag(is, "Has MPI connectivity");
  if (has_mpi) {
    skip_to_line(is, "C-MPI");
    read_mpi(is, m.corners);

    skip_to_line(is, "E-MPI");
    read_mpi(is, m.edges);

    skip_to_line(is, "F-MPI");
    read_mpi(is, m.faces);

    skip_to_line(is, "P-MPI");
    read_mpi(is, m.points);

    skip_to_line(is, "S-MPI");
    read_mpi(is, m.sides);

    skip_to_line(is, "Z-MPI");
    read_mpi(is, m.zones);

    if (m.dump_iotas) {
      skip_to_line(is, "A-MPI");
      read_mpi(is, m.iotas);
    }
  }

  const size_t num_mats = read_tag(is, "Num materials");
  m.zones.subsets.resize(num_mats);
  for (size_t i = 0; i < num_mats; ++i) {
    m.zones.subsets[i].name = read_tag_str(is, "Mat name");
    const size_t num_elements = read_tag(is, "Num mat zones");
    m.zones.subsets[i].lsize = read_tag(is, "Local mat zones");
    m.zones.subsets[i].elements.resize(num_elements);
    for (size_t j = 0; j < num_elements; ++j) {
      is >> m.zones.subsets[i].elements[j];
    }
    m.zones.subsets[i].mask.resize(num_elements);
    for (size_t j = 0; j < num_elements; ++j) {
      is >> m.zones.subsets[i].mask[j];
    }
    is >> std::ws;
  }

  return 0;
}
