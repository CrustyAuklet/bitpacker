#include "test_common.hpp"
#include <array>

/************************  Don't unpack adjacent bits  ************************/

TEST_CASE("Unpack within bytes (unpacked bits are 1)", "[unpack]") {
    const std::array<uint8_t, 3> input{0b11111111, 0b11111111, 0b11111111};
    REQUIRE( bitpacker::extract<uint8_t>(input, 5, 1) == 0b1u);
    REQUIRE( bitpacker::extract<uint16_t>(input, 10, 3) == 0b111u);
    REQUIRE( bitpacker::extract<uint32_t>(input, 17, 2) == 0b11u);
    REQUIRE( bitpacker::extract<uint64_t>(input, 20, 4) == 0b1111u);
}

TEST_CASE("Unpack within bytes (unpacked bits are 0)", "[unpack]") {
    const std::array<uint8_t, 3> input{0b00000100, 0b00111000, 0b01101111};
    REQUIRE( bitpacker::extract<uint8_t>(input, 5, 1) == 0b1u);
    REQUIRE( bitpacker::extract<uint16_t>(input, 10, 3) == 0b111u);
    REQUIRE( bitpacker::extract<uint32_t>(input, 17, 2) == 0b11u);
    REQUIRE( bitpacker::extract<uint64_t>(input, 20, 4) == 0b1111u);
}

/****************  Unpack values aligned with byte boundaries  *****************/

TEST_CASE("Unpack 4-bit value from MSB", "[unpack]") {
    const std::array<uint8_t, 3> input1{0xFF, 0xAF, 0xFF};
    const std::array<uint8_t, 3> input2{0x00, 0xA0, 0x00};
    const unsigned expected = 0xAu;
    REQUIRE( bitpacker::extract<uint8_t>(input1,  8, 4) == expected);
    REQUIRE( bitpacker::extract<uint8_t>(input2,  8, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input1, 8, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 8, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 4) == expected);
}

TEST_CASE("Unpack 4-bit value from LSB", "[unpack]") {
    const std::array<uint8_t, 3> input1{0xFF, 0xFA, 0xFF};
    const std::array<uint8_t, 3> input2{0x00, 0x0A, 0x00};
    const unsigned expected = 0xAu;
    REQUIRE( bitpacker::extract<uint8_t>(input1,  12, 4) == expected);
    REQUIRE( bitpacker::extract<uint8_t>(input2,  12, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input1, 12, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 12, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 12, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 12, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 12, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 12, 4) == expected);
}

TEST_CASE("Unpack aligned 8-bit values ", "[unpack]") {
    const std::array<uint8_t, 3> input1{0xFF, 0x12, 0xFF};
    const std::array<uint8_t, 3> input2{0x00, 0x12, 0x00};
    const unsigned expected = 0x12u;
    REQUIRE( bitpacker::extract<uint8_t>(input1,  8, 8) == expected);
    REQUIRE( bitpacker::extract<uint8_t>(input2,  8, 8) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input1, 8, 8) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 8, 8) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 8) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 8) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 8) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 8) == expected);
}

TEST_CASE("Unpack aligned 12-bit values ", "[unpack]") {
    const std::array<uint8_t, 3> input1{0xFF, 0x12, 0x3F};
    const std::array<uint8_t, 3> input2{0x00, 0x12, 0x30};
    const unsigned expected = 0x123u;
    REQUIRE( bitpacker::extract<uint16_t>(input1, 8, 12) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 8, 12) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 12) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 12) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 12) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 12) == expected);
}

TEST_CASE("Unpack aligned 16-bit values ", "[unpack]") {
    const std::array<uint8_t, 4> input1{0xFF, 0x12, 0x34, 0xFF};
    const std::array<uint8_t, 4> input2{0x00, 0x12, 0x34, 0x00};
    const unsigned expected = 0x1234u;
    REQUIRE( bitpacker::extract<uint16_t>(input1, 8, 16) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 8, 16) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 16) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 16) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 16) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 16) == expected);
}

TEST_CASE("Unpack aligned 20-bit values ", "[unpack]") {
    const std::array<uint8_t, 4> input1{0xFF, 0x12, 0x34, 0x5F};
    const std::array<uint8_t, 4> input2{0x00, 0x12, 0x34, 0x50};
    const unsigned expected = 0x12345ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 20) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 20) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 20) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 20) == expected);
}

TEST_CASE("Unpack aligned 24-bit values ", "[unpack]") {
    const std::array<uint8_t, 5> input1{0xFF, 0x12, 0x34, 0x56, 0xFF};
    const std::array<uint8_t, 5> input2{0x00, 0x12, 0x34, 0x56, 0x00};
    const unsigned expected = 0x123456ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 24) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 24) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 24) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 24) == expected);
}

TEST_CASE("Unpack aligned 28-bit values ", "[unpack]") {
    const std::array<uint8_t, 5> input1{0xFF, 0x12, 0x34, 0x56, 0x7F};
    const std::array<uint8_t, 5> input2{0x00, 0x12, 0x34, 0x56, 0x70};
    const unsigned expected = 0x1234567ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 28) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 28) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 28) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 28) == expected);
}

TEST_CASE("Unpack aligned 32-bit values ", "[unpack]") {
    const std::array<uint8_t, 6> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0xFF};
    const std::array<uint8_t, 6> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x00};
    const unsigned expected = 0x12345678ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 8, 32) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 8, 32) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 32) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 32) == expected);
}

TEST_CASE("Unpack aligned 36-bit values ", "[unpack]") {
    const std::array<uint8_t, 6> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9F};
    const std::array<uint8_t, 6> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
    const uint64_t expected = 0x123456789ull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 36) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 36) == expected);
}

TEST_CASE("Unpack aligned 40-bit values ", "[unpack]") {
    const std::array<uint8_t, 7> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xFF};
    const std::array<uint8_t, 7> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0x00};
    const uint64_t expected = 0x123456789Aull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 40) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 40) == expected);
}

TEST_CASE("Unpack aligned 44-bit values ", "[unpack]") {
    const std::array<uint8_t, 7> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBF};
    const std::array<uint8_t, 7> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xB0};
    const uint64_t expected = 0x123456789ABull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 44) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 44) == expected);
}

TEST_CASE("Unpack aligned 48-bit values ", "[unpack]") {
    const std::array<uint8_t, 8> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xFF};
    const std::array<uint8_t, 8> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x00};
    const uint64_t expected = 0x123456789ABCull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 48) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 48) == expected);
}

TEST_CASE("Unpack aligned 52-bit values ", "[unpack]") {
    const std::array<uint8_t, 8> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDF};
    const std::array<uint8_t, 8> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xD0};
    const uint64_t expected = 0x123456789ABCDull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 52) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 52) == expected);
}

TEST_CASE("Unpack aligned 56-bit values ", "[unpack]") {
    const std::array<uint8_t, 9> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};
    const std::array<uint8_t, 9> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0x00};
    const uint64_t expected = 0x123456789ABCDEull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 56) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 56) == expected);
}

TEST_CASE("Unpack aligned 60-bit values ", "[unpack]") {
    const std::array<uint8_t, 9> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};
    const std::array<uint8_t, 9> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    const uint64_t expected = 0x123456789ABCDEFull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 60) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 60) == expected);
}

TEST_CASE("Unpack aligned 64-bit values ", "[unpack]") {
    const std::array<uint8_t, 10> input1{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1, 0xFF};
    const std::array<uint8_t, 10> input2{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1, 0x00};
    const uint64_t expected = 0x123456789ABCDEF1ull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 8, 64) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 8, 64) == expected);
}

/*******************  Unpack values across byte boundaries  ********************/

TEST_CASE("Unpack 4-bit value across boundary", "[unpack]") {
    const std::array<uint8_t, 3> input1{0b0000'0010, 0b1000'0000};
    const std::array<uint8_t, 3> input2{0b1111'1110, 0b1011'1111};
    const unsigned expected = 0xAu;
    REQUIRE( bitpacker::extract<uint8_t>(input1,  6, 4) == expected);
    REQUIRE( bitpacker::extract<uint8_t>(input2,  6, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input1, 6, 4) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 6, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 6, 4) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 6, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 6, 4) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 6, 4) == expected);
}

TEST_CASE("Unpack 8-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 3> input1{0x01, 0x20};
    const std::array<uint8_t, 3> input2{0xF1, 0x2F};
    const unsigned expected = 0x12u;
    REQUIRE( bitpacker::extract<uint8_t>(input1,  4, 8) == expected);
    REQUIRE( bitpacker::extract<uint8_t>(input2,  4, 8) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input1, 4, 8) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 4, 8) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 8) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 8) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 8) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 8) == expected);
}

TEST_CASE("Unpack 12-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 3> input1{0x01, 0x23, 0x00};
    const std::array<uint8_t, 3> input2{0xF1, 0x23, 0xFF};
    const unsigned expected = 0x123u;
    REQUIRE( bitpacker::extract<uint16_t>(input1, 4, 12) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 4, 12) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 12) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 12) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 12) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 12) == expected);
}

TEST_CASE("Unpack 16-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 3> input1{0x01, 0x23, 0x40};
    const std::array<uint8_t, 3> input2{0xF1, 0x23, 0x4F};
    const unsigned expected = 0x1234u;
    REQUIRE( bitpacker::extract<uint16_t>(input1, 4, 16) == expected);
    REQUIRE( bitpacker::extract<uint16_t>(input2, 4, 16) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 16) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 16) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 16) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 16) == expected);
}

TEST_CASE("Unpack 20-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 4> input1{0x01, 0x23, 0x45, 0x00};
    const std::array<uint8_t, 4> input2{0xF1, 0x23, 0x45, 0xFF};
    const unsigned expected = 0x12345ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 20) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 20) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 20) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 20) == expected);
}

TEST_CASE("Unpack 24-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 4> input1{0x01, 0x23, 0x45, 0x60};
    const std::array<uint8_t, 4> input2{0xF1, 0x23, 0x45, 0x6F};
    const unsigned expected = 0x123456ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 24) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 24) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 24) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 24) == expected);
}

TEST_CASE("Unpack 28-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 5> input1{0x01, 0x23, 0x45, 0x67, 0x00};
    const std::array<uint8_t, 5> input2{0xF1, 0x23, 0x45, 0x67, 0xFF};
    const unsigned expected = 0x1234567ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 28) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 28) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 28) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 28) == expected);
}

TEST_CASE("Unpack 32-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 5> input1{0x01, 0x23, 0x45, 0x67, 0x80};
    const std::array<uint8_t, 5> input2{0xF1, 0x23, 0x45, 0x67, 0x8F};
    const unsigned expected = 0x12345678ul;
    REQUIRE( bitpacker::extract<uint32_t>(input1, 4, 32) == expected);
    REQUIRE( bitpacker::extract<uint32_t>(input2, 4, 32) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 32) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 32) == expected);
}

TEST_CASE("Unpack 36-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 6> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0x00};
    const std::array<uint8_t, 6> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xFF};
    const uint64_t expected = 0x123456789ull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 36) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 36) == expected);
}

TEST_CASE("Unpack 40-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 6> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xA0};
    const std::array<uint8_t, 6> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAF};
    const uint64_t expected = 0x123456789Aull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 40) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 40) == expected);
}

TEST_CASE("Unpack 44-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 7> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x00};
    const std::array<uint8_t, 7> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xFF};
    const uint64_t expected = 0x123456789ABull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 44) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 44) == expected);
}

TEST_CASE("Unpack 48-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 7> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xC0};
    const std::array<uint8_t, 7> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCF};
    const uint64_t expected = 0x123456789ABCull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 48) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 48) == expected);
}

TEST_CASE("Unpack 52-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 8> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0x00};
    const std::array<uint8_t, 8> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xFF};
    const uint64_t expected = 0x123456789ABCDull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 52) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 52) == expected);
}

TEST_CASE("Unpack 56-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 8> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xE0};
    const std::array<uint8_t, 8> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    const uint64_t expected = 0x123456789ABCDEull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 56) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 56) == expected);
}

TEST_CASE("Unpack 60-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 9> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x00};
    const std::array<uint8_t, 9> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFF};
    const uint64_t expected = 0x123456789ABCDEFull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 60) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 60) == expected);
}

TEST_CASE("Unpack 64-bit value across boundaries", "[unpack]") {
    const std::array<uint8_t, 9> input1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x10};
    const std::array<uint8_t, 9> input2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x1F};
    const uint64_t expected = 0x123456789ABCDEF1ull;
    REQUIRE( bitpacker::extract<uint64_t>(input1, 4, 64) == expected);
    REQUIRE( bitpacker::extract<uint64_t>(input2, 4, 64) == expected);
}
