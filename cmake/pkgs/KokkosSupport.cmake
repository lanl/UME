find_package(Kokkos REQUIRED)

set(COMMON_LINK_LIBRARIES Kokkos::kokkos)
# Link CUDART for CUDA backend
if(Kokkos_ENABLE_CUDA)
  find_package(CUDAToolkit REQUIRED)
  list(APPEND COMMON_LINK_LIBRARIES CUDA::cudart)
  message(STATUS "Kokkos CUDA backend detected - linking CUDA::cudart")
endif()

set(COMMON_COMPILE_DEFINITIONS
  ${COMMON_COMPILE_DEFINITIONS}
  )
