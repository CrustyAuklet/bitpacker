# Helper to use conan generated configuration if provided
macro(conan_init generator)
  if(${generator} STREQUAL "cmake_paths")
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake OPTIONAL)
  elseif(${generator} STREQUAL "cmake")
    if(NOT DEFINED CONAN_PACKAGE_NAME)
      if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
        include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
        conan_basic_setup(TARGETS)
      endif()
    endif()
  else()
    message(FATAL_ERROR "Unknown Conan generator: ${generator}")
  endif()
endmacro()