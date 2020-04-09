#define BITPACKER_NO_STD_BYTE
#include "bitpacker/bitpacker.hpp"
#include "test_common.hpp"
#include <array>

TEST_CASE("Count items in a format string", "[format-string]") {
    REQUIRE(bitpacker::countItems(BP_STRING("u5")) == 1);
    REQUIRE(bitpacker::countItems(BP_STRING("u5b2")) == 2);
    REQUIRE(bitpacker::countItems(BP_STRING("t3u1s3b2r3")) == 5);
    REQUIRE(bitpacker::countItems(BP_STRING("t10u1s3b2r3")) == 5);
    REQUIRE(bitpacker::countItems(BP_STRING("t10u1s3b2r15")) == 5);
}

TEST_CASE("parse endianness correctly for one item", "[format-string]") {
    REQUIRE( bitpacker::getTypeOfItem<0>(BP_STRING("u5")).endian == bitpacker::RawFormatType::Endian::big );
    REQUIRE( bitpacker::getTypeOfItem<0>(BP_STRING(">u5")).endian == bitpacker::RawFormatType::Endian::big );
    REQUIRE( bitpacker::getTypeOfItem<0>(BP_STRING("<u5")).endian == bitpacker::RawFormatType::Endian::little );
}

TEST_CASE("parse endianness correctly for many items", "[format-string]") {
    const auto format = BP_STRING("<t3u1>s3b2");
    REQUIRE( bitpacker::getTypeOfItem<0>(format).endian == bitpacker::RawFormatType::Endian::little );
    REQUIRE( bitpacker::getTypeOfItem<1>(format).endian == bitpacker::RawFormatType::Endian::little );
    REQUIRE( bitpacker::getTypeOfItem<2>(format).endian == bitpacker::RawFormatType::Endian::big );
    REQUIRE( bitpacker::getTypeOfItem<3>(format).endian == bitpacker::RawFormatType::Endian::big );
}

TEST_CASE("parse count correctly", "[format-string]") {
    const auto format = BP_STRING("<t3u1>s3b2");
    REQUIRE( bitpacker::getTypeOfItem<0>(format).count == 3 );
    REQUIRE( bitpacker::getTypeOfItem<1>(format).count == 1 );
    REQUIRE( bitpacker::getTypeOfItem<2>(format).count == 3 );
    REQUIRE( bitpacker::getTypeOfItem<3>(format).count == 2 );
}

TEST_CASE("parse bit offset correctly", "[format-string]") {
    const auto format = BP_STRING("<t3u1>s3b2");
    REQUIRE( bitpacker::getBitOffset<0>(format) == 0 );
    REQUIRE( bitpacker::getBitOffset<1>(format) == 3*8 );
    REQUIRE( bitpacker::getBitOffset<2>(format) == (3*8)+1 );
    REQUIRE( bitpacker::getBitOffset<3>(format) == (3*8)+1+3 );
}
