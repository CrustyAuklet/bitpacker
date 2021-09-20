#include "bitpacker/bit.hpp"

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include "test_common.hpp"

TEST_CASE("test is_aligned", "[bit][util]") {
    constexpr std::array<std::tuple<int, bool, bool>, 10> results = {{
        {0, bitpacker::is_aligned(0), true},
        {8, bitpacker::is_aligned(8), true},
        {16, bitpacker::is_aligned(16), true},
        {128, bitpacker::is_aligned(128), true},
        {1024, bitpacker::is_aligned(1024), true},
        {2, bitpacker::is_aligned(2), false},
        {13, bitpacker::is_aligned(13), false},
        {257, bitpacker::is_aligned(257), false},
        {1023, bitpacker::is_aligned(1023), false},
        {123235, bitpacker::is_aligned(123235), false}
    }};

    for (const auto& [n, const_result, expected] : results) {
        INFO("with bit_offset " << n << " we expect "<< std::boolalpha << expected);
        REQUIRE(const_result == expected);
        REQUIRE(bitpacker::is_aligned(n) == expected);
    }
}

TEST_CASE("test right_mask", "[bit][util]") {
    constexpr std::array<std::tuple<size_t, uint8_t, uint8_t>, 9> results = {{
        {0, bitpacker::right_mask(0), 0b11111111},
        {1, bitpacker::right_mask(1), 0b01111111},
        {2, bitpacker::right_mask(2), 0b00111111},
        {3, bitpacker::right_mask(3), 0b00011111},
        {4, bitpacker::right_mask(4), 0b00001111},
        {5, bitpacker::right_mask(5), 0b00000111},
        {6, bitpacker::right_mask(6), 0b00000011},
        {7, bitpacker::right_mask(7), 0b00000001},
        {8, bitpacker::right_mask(8), 0b00000000},
    }};

    for (const auto& [n, const_result, expected] : results) {
        INFO("with bit_offset " << n << " we expect a mask of " << expected);
        REQUIRE(const_result == expected);
        REQUIRE(bitpacker::right_mask(n) == expected);
    }
}

TEST_CASE("test left_mask", "[bit][util]") {
    constexpr std::array<std::tuple<size_t, uint8_t, uint8_t>, 9> results = {{
        {0, bitpacker::left_mask(0), 0b10000000},
        {1, bitpacker::left_mask(1), 0b11000000},
        {2, bitpacker::left_mask(2), 0b11100000},
        {3, bitpacker::left_mask(3), 0b11110000},
        {4, bitpacker::left_mask(4), 0b11111000},
        {5, bitpacker::left_mask(5), 0b11111100},
        {6, bitpacker::left_mask(6), 0b11111110},
        {7, bitpacker::left_mask(7), 0b11111111},
        {8, bitpacker::left_mask(8), 0b11111111},
    }};

    for (const auto& [n, const_result, expected] : results) {
        INFO("with bit_offset " << n << " we expect a mask of " << expected);
        REQUIRE(const_result == expected);
        REQUIRE(bitpacker::left_mask(n) == expected);
    }
}


TEST_CASE("unsigned type alias", "[bit][util]") {
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<0>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<1>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<2>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<3>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<4>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<5>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<6>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<7>, uint8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<8>, uint8_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<9>, uint8_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<8>, uint16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<9>, uint16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<10>, uint16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<15>, uint16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<16>, uint16_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<17>, uint16_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<16>, uint32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<17>, uint32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<18>, uint32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<31>, uint32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<32>, uint32_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<33>, uint32_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<32>, uint64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<33>, uint64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<34>, uint64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<63>, uint64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<64>, uint64_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::unsigned_type<65>, uint64_t>::value);

    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<65>, void>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<128>, void>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::unsigned_type<1023>, void>::value);
}

TEST_CASE("signed type alias", "[bit][util]") {
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<0>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<1>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<2>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<3>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<4>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<5>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<6>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<7>, int8_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<8>, int8_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<9>, int8_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<8>, int16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<9>, int16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<10>, int16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<15>, int16_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<16>, int16_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<17>, int16_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<16>, int32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<17>, int32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<18>, int32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<31>, int32_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<32>, int32_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<33>, int32_t>::value);

    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<32>, int64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<33>, int64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<34>, int64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<63>, int64_t>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<64>, int64_t>::value);
    STATIC_REQUIRE_FALSE(std::is_same<bitpacker::signed_type<65>, int64_t>::value);

    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<65>, void>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<128>, void>::value);
    STATIC_REQUIRE(std::is_same<bitpacker::signed_type<1023>, void>::value);
}
