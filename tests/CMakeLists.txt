find_package(Catch2 QUIET)

add_library(catch_main STATIC test_main.cpp)

if(Catch2_FOUND)
    target_link_libraries(catch_main PUBLIC Catch2::Catch2)
else()
    message("Catch2 not found, installing with Conan...")
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message( STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)
    conan_add_remote(NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan)

    conan_cmake_run(
            REQUIRES catch2/2.11.0
            BASIC_SETUP
            CMAKE_TARGETS # individual targets to link to
            BUILD missing)

    target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)
endif()

add_library(pybitstruct STATIC bitstream.h bitstream.c)

add_executable(bitpacker_tests)
target_link_libraries(bitpacker_tests PRIVATE catch_main bitpacker::bitpacker pybitstruct)

target_sources(bitpacker_tests PRIVATE
    test_compare_to_bitstruct.cpp

    test_common.hpp
    test_pack_impl.cpp
    test_unpack_impl.cpp
    test_specializations.cpp
    test_bitstream.cpp
)

# prevents the adding of catch test projects to our project
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(Catch)

# uses --list-test-names-only of test executable to find test names! great!
catch_discover_tests(
    bitpacker_tests
    EXTRA_ARGS -s --reporter=xml --out=tests.xml
)
