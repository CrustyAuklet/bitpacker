#include "test_common.hpp"
#include <array>

/************************  Don't affect adjacent bits  ************************/

TEST_CASE("Can insert 1s within bytes", "[pack]") {
    std::array<uint8_t, 3>        input{0b00000000, 0b00000000, 0b00000000};
    const std::array<uint8_t, 3> output{0b00000100, 0b00111000, 0b01100000};
    bitpacker::insert(input, 5, 1, 0b1u);
    bitpacker::insert(input, 10, 3, 0b111u);
    bitpacker::insert(input, 17, 2, 0b11u);
    REQUIRE(input == output);
}

TEST_CASE("Can insert 0s within bytes", "[pack]") {
    std::array<uint8_t, 3>        input{0b11111111, 0b11111111, 0b11111111};
    const std::array<uint8_t, 3> output{0b11111101, 0b11000111, 0b10011111};
    bitpacker::insert(input, 6, 1, 0b0u);
    bitpacker::insert(input, 10, 3, 0b0u);
    bitpacker::insert(input, 17, 2, 0b0u);
    REQUIRE(input == output);
}

/************************  Input value larger than expected  ************************/

TEST_CASE("Can insert 0s within bytes and truncate long values", "[pack]") {
    std::array<uint8_t, 3>        input{0b11110101, 0b10111011, 0b01101111};
    const std::array<uint8_t, 3> output{0b11110001, 0b10000011, 0b00001111};
    bitpacker::insert(input, 5, 1, 0xFFFFFFFFu << 1u);
    bitpacker::insert(input, 10, 3, 0xFFFFFFFFu << 3u);
    bitpacker::insert(input, 17, 2, 0xFFFFFFFFu << 2u);
    REQUIRE(input == output);
}

TEST_CASE("Can insert 1s within bytes and truncate long values", "[pack]") {
    std::array<uint8_t, 3>        input2{0b00000000, 0b00000000, 0b00000000};
    const std::array<uint8_t, 3> output2{0b00000100, 0b00111000, 0b01100000};
    bitpacker::insert(input2, 5, 1, 0xFFFFFFFFu);
    bitpacker::insert(input2, 10, 3, 0xFFFFFFFFu);
    bitpacker::insert(input2, 17, 2, 0xFFFFFFFFu);
    REQUIRE(input2 == output2);
}

/************************  Pack bits across byte boundries  ************************/

TEST_CASE("Can insert 1s across boundaries", "[pack]") {
    std::array<uint8_t, 3>        input{0b00000000, 0b00000000, 0b00000000};
    const std::array<uint8_t, 3> output{0b00000001, 0b11111111, 0b10000000};
    bitpacker::insert(input, 7, 10, 0b1'11111111'1u);
    REQUIRE(input == output);
}

TEST_CASE("Can insert zero value across boundries", "[pack]") {
    std::array<uint8_t, 3>        input{0b11111111, 0b11111111, 0b11111111};
    const std::array<uint8_t, 3> output{0b11111110, 0b00000000, 0b00111111};
    bitpacker::insert(input, 7, 11, 0b0u);
    REQUIRE(input == output);
}

/************************  Can insert a zero length value (does nothing)  ************************/

TEST_CASE("Can insert no value", "[pack]") {
    std::array<uint8_t, 3>        input{0b11111111, 0b11111111, 0b11111111};
    const std::array<uint8_t, 3> output{0b11111111, 0b11111111, 0b11111111};
    bitpacker::insert(input, 5, 0, 0u);
    REQUIRE(input == output);
}

/************************  Able to pack byte aligned values  ************************/
TEST_CASE("Can insert 4-bit values in MSB", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x00, 0x50, 0x00};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xFF, 0x5F, 0xFF};

    bitpacker::insert(input1, 8, 4, 0x5u);
    bitpacker::insert(input2, 8, 4, 0x5u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert 4-bit values in LSB", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x00, 0x05, 0x00};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xFF, 0xF5, 0xFF};

    bitpacker::insert(input1, 12, 4, 0x5u);
    bitpacker::insert(input2, 12, 4, 0x5u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 8-bit values", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x00, 0x12, 0x00};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xFF, 0x12, 0xFF};

    bitpacker::insert(input1, 8, 8, 0x12u);
    bitpacker::insert(input2, 8, 8, 0x12u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 12-bit values", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x00, 0x12, 0x30};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xFF, 0x12, 0x3F};

    bitpacker::insert(input1, 8, 12, 0x123u);
    bitpacker::insert(input2, 8, 12, 0x123u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 16-bit values", "[pack]") {
    std::array<      uint8_t, 4>  input1{0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 4> output1{0x00, 0x12, 0x34, 0x00};
    std::array<      uint8_t, 4>  input2{0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 4> output2{0xFF, 0x12, 0x34, 0xFF};

    bitpacker::insert(input1, 8, 16, 0x1234u);
    bitpacker::insert(input2, 8, 16, 0x1234u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 20-bit values", "[pack]") {
    std::array<      uint8_t, 4>  input1{0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 4> output1{0x00, 0x12, 0x34, 0x50};
    std::array<      uint8_t, 4>  input2{0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 4> output2{0xFF, 0x12, 0x34, 0x5F};

    bitpacker::insert(input1, 8, 20, 0x12345ul);
    bitpacker::insert(input2, 8, 20, 0x12345ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 24-bit values", "[pack]") {
    std::array<      uint8_t, 5>  input1{0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 5> output1{0x00, 0x12, 0x34, 0x56, 0x00};
    std::array<      uint8_t, 5>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 5> output2{0xFF, 0x12, 0x34, 0x56, 0xFF};

    bitpacker::insert(input1, 8, 24, 0x123456ul);
    bitpacker::insert(input2, 8, 24, 0x123456ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 28-bit values", "[pack]") {
    std::array<      uint8_t, 5>  input1{0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 5> output1{0x00, 0x12, 0x34, 0x56, 0x70};
    std::array<      uint8_t, 5>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 5> output2{0xFF, 0x12, 0x34, 0x56, 0x7F};

    bitpacker::insert(input1, 8, 28, 0x1234567ul);
    bitpacker::insert(input2, 8, 28, 0x1234567ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 32-bit values", "[pack]") {
    std::array<      uint8_t, 6>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 6> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x00};
    std::array<      uint8_t, 6>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 6> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0xFF};

    bitpacker::insert(input1, 8, 32, 0x12345678ul);
    bitpacker::insert(input2, 8, 32, 0x12345678ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 36-bit values", "[pack]") {
    std::array<      uint8_t, 6>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 6> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x90};
    std::array<      uint8_t, 6>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 6> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9F};

    bitpacker::insert(input1, 8, 36, 0x123456789ull);
    bitpacker::insert(input2, 8, 36, 0x123456789ull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 40-bit values", "[pack]") {
    std::array<      uint8_t, 7>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 7> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0x00};
    std::array<      uint8_t, 7>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 7> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xFF};

    bitpacker::insert(input1, 8, 40, 0x123456789Aull);
    bitpacker::insert(input2, 8, 40, 0x123456789Aull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 44-bit values", "[pack]") {
    std::array<      uint8_t, 7>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 7> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xB0};
    std::array<      uint8_t, 7>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 7> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBF};

    bitpacker::insert(input1, 8, 44, 0x123456789ABull);
    bitpacker::insert(input2, 8, 44, 0x123456789ABull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 48-bit values", "[pack]") {
    std::array<      uint8_t, 8>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 8> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x00};
    std::array<      uint8_t, 8>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 8> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xFF};

    bitpacker::insert(input1, 8, 48, 0x123456789ABCull);
    bitpacker::insert(input2, 8, 48, 0x123456789ABCull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 52-bit values", "[pack]") {
    std::array<      uint8_t, 8>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 8> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xD0};
    std::array<      uint8_t, 8>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 8> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDF};

    bitpacker::insert(input1, 8, 52, 0x123456789ABCDull);
    bitpacker::insert(input2, 8, 52, 0x123456789ABCDull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 56-bit values", "[pack]") {
    std::array<      uint8_t, 9>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 9> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0x00};
    std::array<      uint8_t, 9>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 9> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};

    bitpacker::insert(input1, 8, 56, 0x123456789ABCDEull);
    bitpacker::insert(input2, 8, 56, 0x123456789ABCDEull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 60-bit values", "[pack]") {
    std::array<      uint8_t, 9>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 9> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    std::array<      uint8_t, 9>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 9> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xEF};

    bitpacker::insert(input1, 8, 60, 0x123456789ABCDEFull);
    bitpacker::insert(input2, 8, 60, 0x123456789ABCDEEull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert aligned 64-bit values", "[pack]") {
    std::array<      uint8_t, 10>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 10> output1{0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1, 0x00};
    std::array<      uint8_t, 10>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 10> output2{0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1, 0xFF};

    bitpacker::insert(input1, 8, 64, 0x123456789ABCDEF1ull);
    bitpacker::insert(input2, 8, 64, 0x123456789ABCDEF1ull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

/************************  Able to pack non byte aligned values  ************************/

TEST_CASE("Can insert non-aligned 4-bit values", "[pack]") {
    std::array<      uint8_t, 2>  input1{0x00, 0x00};
    const std::array<uint8_t, 2> output1{0b0000'0001, 0b0100'0000};
    std::array<      uint8_t, 2>  input2{0xFF, 0xFF};
    const std::array<uint8_t, 2> output2{0b1111'1101, 0b0111'1111};

    bitpacker::insert(input1, 6, 4, 0x5u);
    bitpacker::insert(input2, 6, 4, 0x5u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 8-bit values", "[pack]") {
    std::array<      uint8_t, 2>  input1{0x00, 0x00};
    const std::array<uint8_t, 2> output1{0x01, 0x20};
    std::array<      uint8_t, 2>  input2{0xFF, 0xFF};
    const std::array<uint8_t, 2> output2{0xF1, 0x2F};

    bitpacker::insert(input1, 4, 8, 0x12u);
    bitpacker::insert(input2, 4, 8, 0x12u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 12-bit values", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x01, 0x23, 0x00};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xF1, 0x23, 0xFF};

    bitpacker::insert(input1, 4, 12, 0x123u);
    bitpacker::insert(input2, 4, 12, 0x123u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 16-bit values", "[pack]") {
    std::array<      uint8_t, 3>  input1{0x00, 0x00, 0x00};
    const std::array<uint8_t, 3> output1{0x01, 0x23, 0x40};
    std::array<      uint8_t, 3>  input2{0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 3> output2{0xF1, 0x23, 0x4F};

    bitpacker::insert(input1, 4, 16, 0x1234u);
    bitpacker::insert(input2, 4, 16, 0x1234u);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 20-bit values", "[pack]") {
    std::array<      uint8_t, 4>  input1{0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 4> output1{0x01, 0x23, 0x45, 0x00};
    std::array<      uint8_t, 4>  input2{0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 4> output2{0xF1, 0x23, 0x45, 0xFF};

    bitpacker::insert(input1, 4, 20, 0x12345ul);
    bitpacker::insert(input2, 4, 20, 0x12345ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 24-bit values", "[pack]") {
    std::array<      uint8_t, 4>  input1{0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 4> output1{0x01, 0x23, 0x45, 0x60};
    std::array<      uint8_t, 4>  input2{0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 4> output2{0xF1, 0x23, 0x45, 0x6F};

    bitpacker::insert(input1, 4, 24, 0x123456ul);
    bitpacker::insert(input2, 4, 24, 0x123456ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 28-bit values", "[pack]") {
    std::array<      uint8_t, 5>  input1{0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 5> output1{0x01, 0x23, 0x45, 0x67, 0x00};
    std::array<      uint8_t, 5>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 5> output2{0xF1, 0x23, 0x45, 0x67, 0xFF};

    bitpacker::insert(input1, 4, 28, 0x1234567ul);
    bitpacker::insert(input2, 4, 28, 0x1234567ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 32-bit values", "[pack]") {
    std::array<      uint8_t, 5>  input1{0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 5> output1{0x01, 0x23, 0x45, 0x67, 0x80};
    std::array<      uint8_t, 5>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 5> output2{0xF1, 0x23, 0x45, 0x67, 0x8F};

    bitpacker::insert(input1, 4, 32, 0x12345678ul);
    bitpacker::insert(input2, 4, 32, 0x12345678ul);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 36-bit values", "[pack]") {
    std::array<      uint8_t, 6>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 6> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0x00};
    std::array<      uint8_t, 6>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 6> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xFF};

    bitpacker::insert(input1, 4, 36, 0x123456789ull);
    bitpacker::insert(input2, 4, 36, 0x123456789ull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 40-bit values", "[pack]") {
    std::array<      uint8_t, 6>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 6> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xA0};
    std::array<      uint8_t, 6>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 6> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAF};

    bitpacker::insert(input1, 4, 40, 0x123456789Aull);
    bitpacker::insert(input2, 4, 40, 0x123456789Aull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 44-bit values", "[pack]") {
    std::array<      uint8_t, 7>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 7> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x00};
    std::array<      uint8_t, 7>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 7> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xFF};

    bitpacker::insert(input1, 4, 44, 0x123456789ABull);
    bitpacker::insert(input2, 4, 44, 0x123456789ABull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 48-bit values", "[pack]") {
    std::array<      uint8_t, 7>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 7> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xC0};
    std::array<      uint8_t, 7>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 7> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCF};

    bitpacker::insert(input1, 4, 48, 0x123456789ABCull);
    bitpacker::insert(input2, 4, 48, 0x123456789ABCull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 52-bit values", "[pack]") {
    std::array<      uint8_t, 8>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 8> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0x00};
    std::array<      uint8_t, 8>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 8> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xFF};

    bitpacker::insert(input1, 4, 52, 0x123456789ABCDull);
    bitpacker::insert(input2, 4, 52, 0x123456789ABCDull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 56-bit values", "[pack]") {
    std::array<      uint8_t, 8>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 8> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xE0};
    std::array<      uint8_t, 8>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 8> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

    bitpacker::insert(input1, 4, 56, 0x123456789ABCDEull);
    bitpacker::insert(input2, 4, 56, 0x123456789ABCDEull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 60-bit values", "[pack]") {
    std::array<      uint8_t, 9>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 9> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x00};
    std::array<      uint8_t, 9>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 9> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEE, 0xFF};

    bitpacker::insert(input1, 4, 60, 0x123456789ABCDEFull);
    bitpacker::insert(input2, 4, 60, 0x123456789ABCDEEull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}

TEST_CASE("Can insert non-aligned 64-bit values", "[pack]") {
    std::array<      uint8_t, 9>  input1{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<uint8_t, 9> output1{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x10};
    std::array<      uint8_t, 9>  input2{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const std::array<uint8_t, 9> output2{0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x1F};

    bitpacker::insert(input1, 4, 64, 0x123456789ABCDEF1ull);
    bitpacker::insert(input2, 4, 64, 0x123456789ABCDEF1ull);
    REQUIRE(input1 == output1);
    REQUIRE(input2 == output2);
}
