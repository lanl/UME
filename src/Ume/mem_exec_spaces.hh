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
\file Ume/mem_exec_spaces.hh
*/

#ifndef UME_MEM_EXEC_SPACE_HH
#define UME_MEM_EXEC_SPACE_HH

#include <Kokkos_Core.hpp>

/* Define the Host memory space */
using HostSpace = Kokkos::HostSpace;

/* Define the default memory space for memory pool allocations */
#if defined(KOKKOS_ENABLE_CUDA) || defined(KOKKOS_ENABLE_SYCL)
/* This will be CudaUVMSpace or SYCLSharedUSMSpace */
using DefaultMemSpace = Kokkos::SharedSpace;
#elif defined(KOKKOS_ENABLE_HIP)
/* We override the use of HIPManagedSpace here because it requires both
 * operating system and hardware to support and enable the xnack feature. */
using DefaultMemSpace = Kokkos::HIPSpace;
#elif !defined(KOKKOS_ENABLE_OPENACC) && !defined(KOKKOS_ENABLE_OPENMPTARGET)
/* This will be HostSpace */
using DefaultMemSpace = Kokkos::SharedSpace;
#else
/* Default to HostSpace */
using DefaultMemSpace = Kokkos::HostSpace;
#endif

/* Define the default execution spaces */
#if defined(UME_SERIAL)
/* Set all execution spaces to Serial */
using DevExecSpace = Kokkos::Serial;
using HostExecSpace = Kokkos::Serial;
#else
/* Device execution space may default to one of:
 *   Cuda, OpenMPTarget, HIP, SYCL, OpenACC, OpenMP, Threads, HPX, Serial
 * Host execution space may default to one of:
 *   OpenMP, Threads, HPX, Serial */
using DevExecSpace = Kokkos::DefaultExecutionSpace;
using HostExecSpace = Kokkos::DefaultHostExecutionSpace;
#endif

/* Define the default execution memory spaces */
#if defined(UME_SERIAL)
/* Set all execution memory spaces to Serial::HostSpace */
using DevExecMemSpace = Kokkos::Serial::memory_space;
using HostExecMemSpace = Kokkos::Serial::memory_space;
#else
/* Device execution memory space may default to one of:
 *   Cuda::{CudaUVMSpace,CudaSpace}, OpenMPTarget::OpenMPTargetSpace,
 *   HIP::HIPSpace, SYCL::SYCLDeviceUSMSpace, OpenACC::OpenACCSpace,
 *   OpenMP::HostSpace, Threads::HostSpace, HPX::HostSpace, Serial::HostSpace
 * Host execution memory space may default to one of:
 *   OpenMP::HostSpace, Threads::HostSpace, HPX::HostSpace, Serial::HostSpace */
using DevExecMemSpace = Kokkos::DefaultExecutionSpace::memory_space;
using HostExecMemSpace = Kokkos::DefaultHostExecutionSpace::memory_space;
#endif

#endif
