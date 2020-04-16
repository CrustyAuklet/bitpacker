#include "test_common.hpp"
#include <array>

TEST_CASE("reverse bits", "[bitpacker::reverse-bits]") {
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 1 >(1U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 2 >(2U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 3 >(4U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 4 >(8U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 5 >(16U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 6 >(32U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 7 >(64U) == 1U);
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 8 >(128U) == 1U);

    // reverses a pattern in various width types
    REQUIRE(bitpacker::impl::reverse_bits< uint8_t, 5 >(5U) == 20U);
    REQUIRE(bitpacker::impl::reverse_bits< uint16_t, 10 >(0x2BBU) == 0x375U);
    REQUIRE(bitpacker::impl::reverse_bits< uint32_t, 21 >(0x15B95AU) == 0xB53B5U);
    REQUIRE(bitpacker::impl::reverse_bits< uint64_t, 45 >(0xB53B5ADCAD7UL) == 0x1D6A76B5B95AUL);

    // reverses only the bits in the range given
    REQUIRE(bitpacker::impl::reverse_bits< uint16_t, 8 >(0x2BBU) == 0xDDU);
    REQUIRE(bitpacker::impl::reverse_bits< uint16_t, 9 >(0x2BBU) == 0x1BAU);
    REQUIRE(bitpacker::impl::reverse_bits< uint32_t, 18 >(0x15B95AU) == 0x16A76U);
    REQUIRE(bitpacker::impl::reverse_bits< uint64_t, 41 >(0xB53B5ADCAD7UL) == 0x1D6A76B5B95UL);
}
