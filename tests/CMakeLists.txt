find_package(Catch2 REQUIRED)

add_library(catch_main STATIC test_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

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
enable_testing()
include(Catch)

# uses --list-test-names-only of test executable to find test names! great!
catch_discover_tests(
    bitpacker_tests
    EXTRA_ARGS -s --reporter=xml --out=tests.xml
)
