/*!
  \file txt2bin.cc
*/
/*
   This reads a file in the LAP ASCII DefliDump format (see
   $OPUS/DELFI/Output/DelfiDump), and creates a binary file for use
   with LAP_Mesh.hh.
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
    std::cout << "Reading text file \"" << argv[1] << "\"" << std::endl;
    std::ifstream is(argv[1]);
    if (!is) {
      std::cerr << "Couldn't open \"" << argv[1] << "\" for reading"
                << std::endl;
      exit(1);
    }
    Ume::Timer timer;
    timer.start();
    read(is, m);
    timer.stop();
    std::cout << "Text read took " << timer << "\n";
    txt_read_time = timer.seconds();
    is.close();
  }

  {
    std::cout << "Writing binary file \"" << argv[2] << "\"" << std::endl;
    std::ofstream os(argv[2]);
    if (!os) {
      std::cerr << "Couldn't open \"" << argv[2] << "\" for writing"
                << std::endl;
      exit(1);
    }
    Ume::Timer timer;
    timer.start();
    m.write(os);
    timer.stop();
    std::cout << "Binary write took " << timer << "\n";
    os.close();
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
      std::cerr << "DIFF!" << std::endl;
      return 1;
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
  // std::cout << ts << ": " << val << '\n';
  is >> std::ws;
  return val;
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
    const int ndims) {
  pts.resize(kkpl, kkpll);

  int pidx, dummy;
  for (int i = 0; i < kkpl; ++i) {
    pidx = -1;
    is >> pidx;
    if (pidx != i + 1) {
      std::cerr << "Point " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> pts.mask[i] >> dummy >> pts.coord[0][i];
    for (int d = 1; d < ndims; ++d) {
      is >> pts.coord[d][i];
    }
    is >> std::ws;
  }
}

void read_zones(
    std::istream &is, Zones &zones, const int kkzl, const int kkzll) {
  int idx, kkmpiztyp;
  zones.resize(kkzl, kkzll);
  for (int i = 0; i < kkzl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Zone " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> zones.mask[i] >> kkmpiztyp >> std::ws;
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

void read_sides(
    std::istream &is, Sides &sides, const int kksl, const int kksll) {
  int idx, kkmpistyp;
  sides.resize(kksl, kksll);
  for (int i = 0; i < kksl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Side " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> sides.mask[i] >> kkmpistyp;
    is >> sides.z[i] >> sides.p1[i] >> sides.p2[i] >> sides.e[i] >>
        sides.f[i] >> sides.c1[i] >> sides.c2[i] >> sides.s2[i] >>
        sides.s3[i] >> sides.s4[i] >> sides.s5[i] >> std::ws;
    --sides.z[i];
    --sides.p1[i];
    --sides.p2[i];
    --sides.e[i];
    --sides.f[i];
    --sides.c1[i];
    --sides.c2[i];
    --sides.s2[i];
    --sides.s3[i];
    --sides.s4[i];
    --sides.s5[i];
  }
}

void read_edges(
    std::istream &is, Edges &edges, const int kkel, const int kkell) {
  int idx, kkmpietyp;
  edges.resize(kkel, kkell);
  for (int i = 0; i < kkel; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Edge " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> edges.mask[i] >> kkmpietyp >> edges.p1[i] >> edges.p2[i] >> std::ws;
    --edges.p1[i];
    --edges.p2[i];
  }
}

void read_faces(
    std::istream &is, Faces &faces, const int kkfl, const int kkfll) {
  int idx, kkmpiftyp;
  faces.resize(kkfl, kkfll);
  for (int i = 0; i < kkfl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Face " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> faces.mask[i] >> kkmpiftyp >> faces.z1[i] >> faces.z2[i] >> std::ws;
    --faces.z1[i];
    --faces.z2[i];
  }
}

void read_corners(
    std::istream &is, Corners &corners, const int kkcl, const int kkcll) {
  int idx, kkmpictyp;
  corners.resize(kkcl, kkcll);
  for (int i = 0; i < kkcl; ++i) {
    idx = -1;
    is >> idx;
    if (idx != i + 1) {
      std::cerr << "Face " << i + 1 << " read error" << std::endl;
      exit(1);
    }
    is >> corners.mask[i] >> kkmpictyp >> corners.p[i] >> corners.z[i] >>
        std::ws;
    --corners.p[i];
    --corners.z[i];
  }
}

int read(std::istream &is, Mesh &m) {
  m.numpe = read_tag(is, "Number of ranks");
  m.mype = read_tag(is, "Rank");
  int ndims = read_tag(is, "Number of dimensions");
  int igeo = read_tag(is, "Geometry (igeo)");
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
    exit(1);
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
  const int kkpll = read_tag(is, "Dimension: points");
  const int kkpl = read_tag(is, "Number of points");
  const int kkzll = read_tag(is, "Dimension: zones");
  const int kkzl = read_tag(is, "Number of zones");
  const int kksll = read_tag(is, "Dimension: sides");
  const int kksl = read_tag(is, "Number of sides");
  const int kkell = read_tag(is, "Dimension: edges");
  const int kkel = read_tag(is, "Number of edges");
  const int kkfll = read_tag(is, "Dimension: faces");
  const int kkfl = read_tag(is, "Number of faces");
  const int kkcll = read_tag(is, "Dimension: corners");
  const int kkcl = read_tag(is, "Number of corners");
  read_tag(is, "Number of regions");
  read_tag(is, "Number of boundaries");

  expect_line(is, "Points");
  skip_line(is);
  read_points(is, m.points, kkpl, kkpll, ndims);

  skip_to_line(is, "Zones");
  skip_line(is);
  read_zones(is, m.zones, kkzl, kkzll);

  skip_to_line(is, "Sides");
  skip_line(is);
  read_sides(is, m.sides, kksl, kksll);

  skip_to_line(is, "Edges");
  skip_line(is);
  read_edges(is, m.edges, kkel, kkell);

  skip_to_line(is, "Faces");
  skip_line(is);
  read_faces(is, m.faces, kkfl, kkfll);

  skip_to_line(is, "Corners");
  skip_line(is);
  read_corners(is, m.corners, kkcl, kkcll);

  // We are not extracting the region or boundary information that is also
  // in the DelfiDump format.

  return 0;
}
