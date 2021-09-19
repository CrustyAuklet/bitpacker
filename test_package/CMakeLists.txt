cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(example example.cpp)
message(STATUS "bitpacker test package imports: ${CONAN_LIBS}")
target_link_libraries(example ${CONAN_LIBS})
set_property(TARGET example PROPERTY CXX_STANDARD 17)
