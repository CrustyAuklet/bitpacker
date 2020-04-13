#define CATCH_CONFIG_ENABLE_PAIR_STRINGMAKER
#include "test_common.hpp"
#include "constexpr_helpers.h"
#include <array>
#include <utility>

namespace bpimpl = bitpacker::impl;

TEST_CASE("consume number from format string", "[format]") {
    using rtype = std::pair<size_t, size_t>;
    REQUIRE_STATIC(bpimpl::consume_number("123", 0) == rtype(123, 3));
    REQUIRE_STATIC(bpimpl::consume_number("c", 0) == rtype(0, 0));
    REQUIRE_STATIC(bpimpl::consume_number("c12345c", 1) == rtype(12345, 6));
    REQUIRE_STATIC(bpimpl::consume_number("c12345c", 0) == rtype(0, 0));
}

TEST_CASE("Count items without endian", "[format]") {
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("u5")) == 1);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("u5b2")) == 2);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("t3u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("t10u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("t10u1s3b2r15")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("f16u1s3b2P8")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("b1u1s3b2p5")) == 5);
}

TEST_CASE("Count items with endian", "[format]") {
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("<u5")) == 1);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("<u5>b2")) == 2);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("<t3>u1<s3>b2<r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("<t10u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("<t10u1>s3b2r15")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("f16u1s3b2P8>")) == 5);
    REQUIRE_STATIC(bpimpl::count_items(BP_STRING("b1u1s3b2p5<")) == 5);
}

TEST_CASE("parse endianness correctly for one item", "[format]") {
    REQUIRE_STATIC( bpimpl::get_item_type<0>(BP_STRING("u5")).endian == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_item_type<0>(BP_STRING(">u5")).endian == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_item_type<0>(BP_STRING("<u5")).endian == bpimpl::Endian::little );
}

TEST_CASE("parse endianness correctly for many items", "[format]") {
    const auto format = BP_STRING("<t3u1>s3b2");
    REQUIRE_STATIC( bpimpl::get_item_type<0>(format).endian == bpimpl::Endian::little );
    REQUIRE_STATIC( bpimpl::get_item_type<1>(format).endian == bpimpl::Endian::little );
    REQUIRE_STATIC( bpimpl::get_item_type<2>(format).endian == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_item_type<3>(format).endian == bpimpl::Endian::big );
}

TEST_CASE("Read byte order for format string correctly", "[format]") {
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("u5")) == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("u5>")) == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("u5<")) == bpimpl::Endian::little );
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("s12p5u5")) == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("f32b5u5>")) == bpimpl::Endian::big );
    REQUIRE_STATIC( bpimpl::get_byte_order(BP_STRING("f64s7u5s3<")) == bpimpl::Endian::little );
}

TEST_CASE("get type of item", "[format]") {
    REQUIRE_STATIC(bpimpl::get_item_type<0>(BP_STRING("u4f16b2s12t10r10f32p2P2")).formatChar == 'u');
    REQUIRE_STATIC(bpimpl::get_item_type<1>(BP_STRING("u4f16b2<s12t10r10f32p2P2<")).formatChar == 'f');
    REQUIRE_STATIC(bpimpl::get_item_type<2>(BP_STRING("u4f16b2s12t10r10f32p2P2")).formatChar == 'b');
    REQUIRE_STATIC(bpimpl::get_item_type<3>(BP_STRING("u4f16b2s12t10r10f32p2P2>")).formatChar == 's');
    REQUIRE_STATIC(bpimpl::get_item_type<4>(BP_STRING("<u4f16b2s12t10r10f32p2P2")).formatChar == 't');
    REQUIRE_STATIC(bpimpl::get_item_type<5>(BP_STRING("u4f16b2s12t10r10<f32p2P2")).formatChar == 'r');
    REQUIRE_STATIC(bpimpl::get_item_type<6>(BP_STRING("u4f16b2s12t10r10f32p2P2")).formatChar == 'f');
    REQUIRE_STATIC(bpimpl::get_item_type<7>(BP_STRING("u4f16<b2s12t10r10f32p2P2")).formatChar == 'p');
    REQUIRE_STATIC(bpimpl::get_item_type<8>(BP_STRING("u4<f16b2>s12t10r10f32p2P2")).formatChar == 'P');
}

TEST_CASE("parse bit count correctly", "[format]") {
    REQUIRE_STATIC(bpimpl::get_item_type<0>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).count == 4 );
    REQUIRE_STATIC(bpimpl::get_item_type<1>(BP_STRING("u4f16b2<s12t10r10f32p2P2<")).count == 16 );
    REQUIRE_STATIC(bpimpl::get_item_type<2>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).count == 2 );
    REQUIRE_STATIC(bpimpl::get_item_type<3>(BP_STRING("u4f16b2s12t10r10f32p2P2>") ).count == 12 );
    REQUIRE_STATIC(bpimpl::get_item_type<4>(BP_STRING("<u4f16b2s12t10r10f32p2P2") ).count == 10 );
    REQUIRE_STATIC(bpimpl::get_item_type<5>(BP_STRING("u4f16b2s12t10r10<f32p2P2") ).count == 10 );
    REQUIRE_STATIC(bpimpl::get_item_type<6>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).count == 32 );
    REQUIRE_STATIC(bpimpl::get_item_type<7>(BP_STRING("u4f16<b2s12t10r10f32p2P2") ).count == 2 );
    REQUIRE_STATIC(bpimpl::get_item_type<8>(BP_STRING("u4<f16b2>s12t10r10f32p2P2")).count == 2 );
}

TEST_CASE("parse bit offset correctly", "[format]") {
    REQUIRE_STATIC(bpimpl::get_item_type<0>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).offset == 0 );
    REQUIRE_STATIC(bpimpl::get_item_type<1>(BP_STRING("u4f16b2<s12t10r10f32p2P2<")).offset == 4 );
    REQUIRE_STATIC(bpimpl::get_item_type<2>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).offset == 4+16 );
    REQUIRE_STATIC(bpimpl::get_item_type<3>(BP_STRING("u4f16b2s12t10r10f32p2P2>") ).offset == 4+16+2 );
    REQUIRE_STATIC(bpimpl::get_item_type<4>(BP_STRING("<u4f16b2s12t10r10f32p2P2") ).offset == 4+16+2+12 );
    REQUIRE_STATIC(bpimpl::get_item_type<5>(BP_STRING("u4f16b2s12t10r10<f32p2P2") ).offset == 4+16+2+12+(8*10) );
    REQUIRE_STATIC(bpimpl::get_item_type<6>(BP_STRING("u4f16b2s12t10r10f32p2P2")  ).offset == 4+16+2+12+(8*10)+(8*10) );
    REQUIRE_STATIC(bpimpl::get_item_type<7>(BP_STRING("u4f16<b2s12t10r10f32p2P2") ).offset == 4+16+2+12+(8*10)+(8*10)+32 );
    REQUIRE_STATIC(bpimpl::get_item_type<8>(BP_STRING("u4<f16b2>s12t10r10f32p2P2")).offset == 4+16+2+12+(8*10)+(8*10)+32+2 );
}

TEST_CASE("calculate format size", "[format]") {
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2<s12t10r10f32p2P2<")) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2>") ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("<u4f16b2s12t10r10f32p2P2") ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10<f32p2P2") ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16<b2s12t10r10f32p2P2") ) == 230 );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4<f16b2>s12t10r10f32p2P2")) == 230 );
}
