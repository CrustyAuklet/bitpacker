#include "test_common.hpp"
#include <array>

TEST_CASE("pack and unpack int8_t", "[pack][unpack][specialize]") {
    std::array<uint8_t, 2> buff{0x00, 0x00};
    const int8_t input = -5;
    bitpacker::store(buff, 4, input);
    REQUIRE(input == bitpacker::get<int8_t>(buff, 4));
    REQUIRE( (buff.front() & 0xF0u) == 0u );
    REQUIRE( (buff.back() & 0x0Fu) == 0u );
}

TEST_CASE("pack and unpack int16_t", "[pack][unpack][specialize]") {
    std::array<uint8_t, 3> buff{0x00, 0x00, 0x00};
    const int16_t input = -1234;
    bitpacker::store(buff, 4, input);
    REQUIRE(input == bitpacker::get<int16_t>(buff, 4));
    REQUIRE( (buff.front() & 0xF0u) == 0u );
    REQUIRE( (buff.back() & 0x0Fu) == 0u );
}

TEST_CASE("pack and unpack int32_t", "[pack][unpack][specialize]") {
    std::array<uint8_t, 5> buff{0x00, 0x00, 0x00, 0x00, 0x00};
    const int32_t input = -1234567891l;
    bitpacker::store(buff, 4, input);
    REQUIRE(input == bitpacker::get<int32_t>(buff, 4));
    REQUIRE( (buff.front() & 0xF0u) == 0u );
    REQUIRE( (buff.back() & 0x0Fu) == 0u );
}

TEST_CASE("pack and unpack int64_t", "[pack][unpack][specialize]") {
    std::array<uint8_t, 9> buff{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const int64_t input = -1234567891234567891ll;
    bitpacker::store(buff, 4, input);
    REQUIRE(input == bitpacker::get<int64_t>(buff, 4));
    REQUIRE( (buff.front() & 0xF0u) == 0u );
    REQUIRE( (buff.back() & 0x0Fu) == 0u );
}

#include <complex>

namespace bitpacker {
    template <>
    constexpr void store(span<bitpacker::byte_type> buffer, std::size_t offset, std::complex<double> value) noexcept {
        bitpacker::store(buffer, offset, static_cast<int16_t>(value.real()));
        bitpacker::store(buffer, offset+16, static_cast<int16_t>(value.imag()));
    }

    template <>
    constexpr std::complex<double> get(span<const bitpacker::byte_type> buffer, std::size_t offset) noexcept {
        const double r = static_cast<int16_t>(unpack_from<uint16_t>(buffer, offset, 16));
        const double i = static_cast<int16_t>(unpack_from<uint16_t>(buffer, offset+16, 16));
        return std::complex<double>{r, i};
    }
}

TEST_CASE("Specialize pack/unpack on a complex type", "[pack][unpack][specialize]") {
    std::array<uint8_t, 9> buff{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::complex<double> input{4.0, 8.0};
    bitpacker::store(buff, 4, input);

    auto unpacked = bitpacker::get<std::complex<double>>(buff, 4);
    REQUIRE(input == unpacked);
}