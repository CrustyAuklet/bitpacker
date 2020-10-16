
#include <catch2/catch.hpp>

#include "span.hpp"
namespace bytepacker {
    using nonstd::span;
}
#include "bitpacker/bytepack.hpp"
#include <array>

TEST_CASE("Unpack within one byte", "[unpack][bytepacker]") {
    constexpr std::array<uint8_t, 4> input{0, 3, 2, 0xFFU };
    REQUIRE( bytepacker::extract<uint8_t>(input, 0) == 0);
    REQUIRE( bytepacker::extract<uint8_t>(input, 1) == 3u);
    REQUIRE( bytepacker::extract<uint8_t>(input, 2) == 2u);
    REQUIRE( bytepacker::extract<uint8_t>(input, 3) == 255u);
    REQUIRE( bytepacker::extract<int8_t>(input, 3) == -1);
}

TEST_CASE("Unpack little endian values", "[unpack][bytepacker]") {
    constexpr std::array<uint8_t, 8> input{0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x02 };
    REQUIRE( bytepacker::extract<uint16_t>(input, 0) == 0x0B0A);
    REQUIRE( bytepacker::extract<uint16_t>(input, 1) == 0x0C0B);
    REQUIRE( bytepacker::extract<uint16_t>(input, 2) == 0x0D0C);
    REQUIRE( bytepacker::extract<uint32_t>(input, 0) == 0x0D0C0B0A);
    REQUIRE( bytepacker::extract<uint32_t>(input, 4) == 0x02010F0E);
    REQUIRE( bytepacker::extract<uint64_t>(input, 0) == 0x02010F0E0D0C0B0A);
}

TEST_CASE("Unpack big endian values", "[unpack][bytepacker]") {
    constexpr std::array<uint8_t, 8> input{0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x02 };
    REQUIRE( bytepacker::extract<uint16_t, false>(input, 0) == 0x0A0B);
    REQUIRE( bytepacker::extract<uint16_t, false>(input, 1) == 0x0B0C);
    REQUIRE( bytepacker::extract<uint16_t, false>(input, 2) == 0x0C0D);
    REQUIRE( bytepacker::extract<uint32_t, false>(input, 0) == 0x0A0B0C0D);
    REQUIRE( bytepacker::extract<uint32_t, false>(input, 4) == 0x0E0F0102);
    REQUIRE( bytepacker::extract<uint64_t, false>(input, 0) == 0x0A0B0C0D0E0F0102);
}