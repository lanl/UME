# Ume/src/CMakeLists.txt

add_subdirectory(Ume)

if(Scoria_REQUIRE_CLIENTS)
  message(STATUS "Building Ume Serial with Scoria")

  add_executable(txt2bin txt2bin.cc ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  target_compile_definitions(txt2bin PUBLIC USE_SCORIA)

  add_executable(ume_serial ume_serial.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  target_compile_definitions(ume_serial PUBLIC USE_SCORIA)

  add_executable(txt2bin_client txt2bin.cc ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  target_compile_definitions(txt2bin_client PUBLIC USE_SCORIA USE_CLIENT)

  add_executable(ume_serial_client ume_serial.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
  target_compile_definitions(ume_serial_client PUBLIC USE_SCORIA USE_CLIENT)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(txt2bin Ume shm pthread)
    target_link_libraries(ume_serial Ume shm pthread)
    target_link_libraries(txt2bin_client UmeClient shm pthread)
    target_link_libraries(ume_serial_client UmeClient shm pthread)
  else()
    target_link_libraries(txt2bin Ume shm pthread rt)
    target_link_libraries(ume_serial Ume shm pthread rt)
    target_link_libraries(txt2bin_client UmeClient shm pthread rt)
    target_link_libraries(ume_serial_client UmeClient shm pthread rt)
   endif()
  
  target_compile_options(txt2bin PRIVATE ${WARNING_FLAGS}
    ${COMMON_COMPILE_OPTIONS})

  set_target_properties(ume_serial PROPERTIES
     COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
     COMPILE_OPTIONS "${WARNING_FLAGS}"
     )

  target_compile_options(txt2bin_client PRIVATE ${WARNING_FLAGS}
    ${COMMON_COMPILE_OPTIONS})

  set_target_properties(ume_serial_client PROPERTIES
     COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
     COMPILE_OPTIONS "${WARNING_FLAGS}"
     )
else()
  add_executable(txt2bin txt2bin.cc)
  target_link_libraries(txt2bin Ume)
  target_compile_options(txt2bin PRIVATE ${WARNING_FLAGS}
    ${COMMON_COMPILE_OPTIONS})

  add_executable(ume_serial ume_serial.cc shm_allocator.hh)
  target_link_libraries(ume_serial Ume)
  set_target_properties(ume_serial PROPERTIES
    COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
    COMPILE_OPTIONS "${WARNING_FLAGS}"
    )
endif()


#############################################################################
# Things that depend on MPI
#############################################################################
if(USE_MPI)
	if(Scoria_REQUIRE_CLIENTS AND USE_CALIPER)
		message(STATUS "Building Ume MPI with Scoria and Caliper Profiling")
		add_executable(ume_mpi ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
		target_include_directories(ume_mpi PUBLIC ${caliper_INCLUDE_DIR})
		target_compile_definitions(ume_mpi PUBLIC USE_SCORIA USE_CALI )

		add_executable(ume_mpi_client ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
		target_include_directories(ume_mpi_client PUBLIC ${caliper_INCLUDE_DIR})
		target_compile_definitions(ume_mpi_client PUBLIC USE_SCORIA USE_CALI USE_CLIENT)

		if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			target_link_libraries(ume_mpi Ume shm pthread caliper)
			target_link_libraries(ume_mpi UmeClient shm pthread caliper)
		else()
			target_link_libraries(ume_mpi Ume shm pthread rt caliper)
			target_link_libraries(ume_mpi_client UmeClient shm pthread rt caliper)
		endif()

		set_target_properties(ume_mpi PROPERTIES
			COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
			COMPILE_OPTIONS "${WARNING_FLAGS}"
			)

		set_target_properties(ume_mpi_client PROPERTIES
			COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
			COMPILE_OPTIONS "${WARNING_FLAGS}"
			)
	elseif(USE_CALIPER)
		message(STATUS "Building Baseline Ume MPI with Caliper Profiling")
		add_executable(ume_mpi ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
		target_include_directories(ume_mpi PUBLIC ${caliper_INCLUDE_DIR})
		target_compile_definitions(ume_mpi PUBLIC USE_CALI)

		if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			target_link_libraries(ume_mpi Ume pthread caliper)
			target_link_libraries(ume_mpi UmeClient pthread caliper)
		else()
			target_link_libraries(ume_mpi Ume pthread rt caliper)
		endif()

		set_target_properties(ume_mpi PROPERTIES
			COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
			COMPILE_OPTIONS "${WARNING_FLAGS}"
			)

	elseif(Scoria_REQUIRE_CLIENTS)
		message(STATUS "Building Ume MPI with Scoria")

		add_executable(ume_mpi ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
		target_compile_definitions(ume_mpi PUBLIC USE_SCORIA)

		add_executable(ume_mpi_client ume_mpi.cc shm_allocator.hh ${CLIENT_SOURCE_FILES} ${SHARED_SOURCE_FILES})
		target_compile_definitions(ume_mpi_client PUBLIC USE_SCORIA USE_CLIENT)

		if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			target_link_libraries(ume_mpi Ume shm pthread)
			target_link_libraries(ume_mpi UmeClient shm pthread)
		else()
			target_link_libraries(ume_mpi Ume shm pthread rt)
			target_link_libraries(ume_mpi_client UmeClient shm pthread rt)
		endif()

		set_target_properties(ume_mpi PROPERTIES
			COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
			COMPILE_OPTIONS "${WARNING_FLAGS}"
			)

		set_target_properties(ume_mpi_client PROPERTIES
			COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
			COMPILE_OPTIONS "${WARNING_FLAGS}"
			)
  else()
    add_executable(ume_mpi ume_mpi.cc shm_allocator.hh)
    target_link_libraries(ume_mpi Ume)
    set_target_properties(ume_mpi PROPERTIES
      COMPILE_DEFINITIONS "${COMMON_COMPILE_DEFINITIONS}"
      COMPILE_OPTIONS "${WARNING_FLAGS}"
      )
  endif()
endif()


#############################################################################
# Things that depend on Kokkos
#############################################################################

if(USE_KOKKOS)
  add_executable(kokkos_hello kokkos_hello.cc)
  target_link_libraries(kokkos_hello
    Ume
    Kokkos::kokkos
    )
endif()


#############################################################################
# Things that depend on OpenACC
#############################################################################

if(USE_OPENACC)
  add_executable(openacc_hello openacc_hello.cc)
  target_link_libraries(openacc_hello
    Ume
    OpenACC::OpenACC_CXX
    )
endif()
