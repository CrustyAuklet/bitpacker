cmake_minimum_required(VERSION 3.15)
project(bitpacker
    VERSION 0.1.0
    DESCRIPTION "Bit level serialization"
    LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_library(bitpacker INTERFACE)
add_library(bitpacker::bitpacker ALIAS bitpacker)
target_compile_features(bitpacker INTERFACE cxx_std_17)

target_sources(bitpacker INTERFACE
    include/bitpacker/bitpacker.hpp
    include/nonstd/span.hpp
)

target_include_directories(bitpacker INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

## packaging and testing: probably only want if this is not a sub-project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(ENABLE_TESTING "Enable Test Builds" ON)
    if(ENABLE_TESTING)
        enable_testing()
        message("Building Tests...")
        add_subdirectory(tests)
    endif()

    #add_subdirectory(packaging)
endif()
