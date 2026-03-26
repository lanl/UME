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

#ifndef UME_MEM_EXEC_SPACES_HH
#define UME_MEM_EXEC_SPACES_HH

/* **NOTES ON MEMORY SPACES**
 *   The default memory space used for scratch array allocations is always
 * non-page-migrating device memory in the HIP GPU and SYCL GPU programming
 * environments. The default memory space used in the CUDA programming
 * environment is the unified virtual memory space CudaUVMSpace. This is because
 * the page-migrating memory spaces HIPManagedSpace and SYCLSharedUSMSpace
 * suffer severe performance degradation unless the XNACK feature is enabled.
 * There is an enormous amount of prerequisites at the hardware and software
 * level that must be met for the XNACK feature to be truly enabled for this
 * appliation:
 * 1) The target device hardware must include the xnack+ target feature.
 *    (ex: amdgcn-amd-amdhsa--gfx942:sramecc+:xnack+)
 * 2) The xnack target feature should be enabled in the --offload-arch compiler
 *    option. This should be set as a pass-through option from Kokkos but it's
 *    unclear how to achieve this in a consistent way that also prevents
 *    multiple offload architectures from being defined.
 * 3) The kernel must support
 *    [Heterogeneous Memory Management (HMM)](https://www.kernel.org/doc/html/v5.0/vm/hmm.html)
 *    and XNACK must be enabled by the kernel.
 * 4) HSA_XNACK=1 must be set in the environment.
 * TODO: figure out the simplest and most portable way to enable the XNACK
 * feature when available. This will allow us to use the shared memory spaces
 * HIPManagedSpace and SYCLSharedUSMSpace in the HIP and SYCL GPU programming
 * environments without severe performance impact. This will also allow us to
 * do things like embed ASAN symbols in device code which currently cannot be
 * done without XNACK (for code development, debugging, and validation).
 */

#include <Kokkos_Core.hpp>

/* Define the Host memory space */
using HostSpace = Kokkos::HostSpace;

/* Define the default memory space for memory pool allocations */
#if defined(UME_SERIAL)
using DefaultMemSpace = Kokkos::Serial::memory_space;
#elif defined(KOKKOS_ENABLE_CUDA)
#if !defined(KOKKOS_HAS_SHARED_SPACE)
using DefaultMemSpace = Kokkos::CudaSpace;
#else
using DefaultMemSpace = Kokkos::CudaUVMSpace;
#endif
#elif defined(KOKKOS_ENABLE_HIP)
using DefaultMemSpace = Kokkos::HIPSpace;
#elif defined(KOKKOS_ENABLE_SYCL)
using DefaultMemSpace = Kokkos::SYCLDeviceUSMSpace;
#else
using DefaultMemSpace = HostSpace;
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
