/*!
  \file Ume/utils.cc
*/

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

namespace Ume {

void debug_attach_point(int const mype) {
  int release = 0;
  char const *const envvalue = std::getenv("UME_DEBUG_RANK");
  if (envvalue == nullptr)
    return;
  int const stoppe = std::stoi(envvalue, nullptr, 10);
  if (stoppe == mype) {
    std::cerr << "Execution is paused on rank " << stoppe << " PID " << getpid()
              << " because the environment variable\nUME_DEBUG_RANK is set. "
              << "If you are debugging a segfault, add the\n"
              << "\"-mca orte_abort_on_non_zero_status 0\" option to mpirun."
              << std::endl;

    while (!release) {
      sleep(5);
    }
  }
#ifdef HAVE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
}

int init_depth(int const delta) {
  static int depth = 0;
  return depth += delta;
}

} // namespace Ume
