
option(USE_OPENACC "Enable support for OpenACC code")

if(USE_OPENACC)
  find_package(OpenACC REQUIRED)
endif()
