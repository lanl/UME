option(USE_CALIPER "Enable support for caliper profiling")

if(USE_CALIPER)
	find_package(caliper)
endif()
