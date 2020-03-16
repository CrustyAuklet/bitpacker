#include <array>
#include <cstddef>
#include "bitpacker/bitpacker.hpp"
#include "test_common.hpp"

namespace bp = bitpacker;
//
//TEST_CASE() {
//    std::array<uint8_t, 8> buf{};
//    bp::BitStream bs{buf};
//    bs.pack_field(0, 10, 44);
//}

//TEST_CASE() {
//    const std::array<uint8_t, 8> buf{};
//    bp::BitStream bs{buf};
//}

//TEST_CASE() {
//    uint8_t buf[8] = {};
//    bp::BitStream bs{buf};
//    bs.pack_field(0, 10, 44);
//}
//
//TEST_CASE() {
//    const uint8_t buf[8] = {};
//    bp::BitStream bs{buf};
//    bs.unpack_field<uint16_t>(0, 10);
//    //bs.pack_field(0, 10, 44);
//}

//TEST_CASE() {
//    uint8_t buf[8] = {};
//    nonstd::span b{buf};
//    bp::BitStream bs{b};
//    bs.pack_field(0, 10, 44);
//}
//
//TEST_CASE() {
//    const uint8_t buf[8] = {};
//    const nonstd::span b{buf};
//    bp::BitStream bs{b};
//}