find_package(Catch2 REQUIRED)
find_package(span-lite REQUIRED)

add_library(catch_main STATIC )
target_compile_features(catch_main PUBLIC cxx_std_17)
target_sources(catch_main PUBLIC
        test_main.cpp
        test_common.hpp
        constexpr_helpers.h
        )
target_link_libraries(catch_main PUBLIC Catch2::Catch2 nonstd::span-lite)

add_executable(bitpacker_test_bit_utils)
target_link_libraries(bitpacker_test_bit_utils PRIVATE catch_main bitpacker::bitpacker)
target_sources(bitpacker_test_bit_utils PRIVATE
        bit.test.cpp)

add_executable(bitpacker_test_core)
target_link_libraries(bitpacker_test_core PRIVATE catch_main bitpacker::bitpacker)
target_sources(bitpacker_test_core PRIVATE
    test_pack_impl.cpp
    test_unpack_impl.cpp
    test_helpers.cpp
)

add_library(pybitstruct STATIC bitstream.h bitstream.c)
add_executable(bitpacker_test_against_bitstruct)
target_link_libraries(bitpacker_test_against_bitstruct PRIVATE catch_main bitpacker::bitpacker pybitstruct)
target_sources(bitpacker_test_against_bitstruct PRIVATE
        test_compare_to_bitstruct.cpp
        )

if(BITPACKER_USE_CXX17)
    message("Building c++17 tests...")
    add_executable(bitpacker_test_against_python)
    target_link_libraries(bitpacker_test_against_python PRIVATE catch_main bitpacker::bitpacker)
    target_sources(bitpacker_test_against_python PRIVATE
            python_common.hpp
            test_bincompat_integers.cpp
            test_bincompat_arrays.cpp
            test_bincompat_into_integers.cpp
            test_bincompat_into_arrays.cpp
        )

    add_executable(bitpacker_test_tmp_helpers)
    target_link_libraries(bitpacker_test_tmp_helpers PRIVATE catch_main bitpacker::bitpacker)
    target_sources(bitpacker_test_tmp_helpers PRIVATE
            test_tmp_formats.cpp
        )
endif()

# prevents the adding of catch test projects to our project
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(Catch)

catch_discover_tests(bitpacker_test_core
    EXTRA_ARGS -s --reporter=xml --out=tests.xml
    )
catch_discover_tests(bitpacker_test_against_bitstruct
    EXTRA_ARGS -s --reporter=xml --out=tests.xml
    )

if(BITPACKER_USE_CXX17)
    catch_discover_tests(bitpacker_test_against_python
        EXTRA_ARGS -s --reporter=xml --out=tests.xml
        )
    catch_discover_tests(bitpacker_test_tmp_helpers
        EXTRA_ARGS -s --reporter=xml --out=tests.xml
        )
endif()
