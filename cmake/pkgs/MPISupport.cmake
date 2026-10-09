
option(USE_MPI "Enable support for MPI code")

if(USE_MPI)
  find_package(MPI REQUIRED)
  list(APPEND COMMON_LINK_LIBRARIES MPI::MPI_CXX)
  set(COMMON_COMPILE_DEFINITIONS
    ${COMMON_COMPILE_DEFINITIONS}
    HAVE_MPI
    )
endif()
