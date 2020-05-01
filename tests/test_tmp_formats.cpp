#define CATCH_CONFIG_ENABLE_PAIR_STRINGMAKER
#include "test_common.hpp"
#include "constexpr_helpers.h"
#include <array>
#include <utility>

namespace bpimpl = bitpacker::impl;

TEST_CASE("consume number from format string", "[format]") {
    using rtype = std::pair<bitpacker::size_type, bitpacker::size_type>;
    REQUIRE_STATIC(bpimpl::consume_number("123", 0) == rtype(123, 3));
    REQUIRE_STATIC(bpimpl::consume_number("c", 0) == rtype(0, 0));
    REQUIRE_STATIC(bpimpl::consume_number("c12345c", 1) == rtype(12345, 6));
    REQUIRE_STATIC(bpimpl::consume_number("c12345c", 0) == rtype(0, 0));
}

TEST_CASE("Count items without endian", "[format]") {
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("u5")) == 1);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("u5b2")) == 2);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("t3u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("t10u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("t10u1s3b2r15")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("f16u1s3b2P8")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("b1u1s3b2p5")) == 5);
}

TEST_CASE("Count items with endian", "[format]") {
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("<u5")) == 1);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("<u5>b2")) == 2);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("<t3>u1<s3>b2<r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("<t10u1s3b2r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("<t10u1>s3b2r15")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("f16u1s3b2P8>")) == 5);
    REQUIRE_STATIC(bpimpl::count_all_items(BP_STRING("b1u1s3b2p5<")) == 5);
}

TEST_CASE("Count items ignore padding", "[format]")
{
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("<u5p5")) == 1);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("<u5P6>b2")) == 2);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("p3<t3>u1<s3P9>b2<r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("<t10u1s3b2p9p6r3")) == 5);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("P10<t10u1>s3b2r15")) == 5);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("f16p1s3b2P8>")) == 3);
    REQUIRE_STATIC(bpimpl::count_non_padding(BP_STRING("b1u1s3b2p5<")) == 4);
}

TEST_CASE("Count items only padding", "[format]")
{
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("<u5p5")) == 1);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("<u5P6>b2")) == 1);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("p3<t3>u1<s3P9>b2<r3")) == 2);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("<t10u1s3b2p9p6r3")) == 2);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("P10<t10u1>s3b2r15")) == 1);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("f16p1s3b2P8>")) == 2);
    REQUIRE_STATIC(bpimpl::count_padding(BP_STRING("b1u1s3b2p5<")) == 1);
}

TEST_CASE("parse endianness correctly for one item", "[format]") {
    REQUIRE_STATIC( bitpacker::impl::get_type_array(BP_STRING("u5" ))[0].endian == bpimpl::Endian::big );
    REQUIRE_STATIC( bitpacker::impl::get_type_array(BP_STRING(">u5"))[0].endian == bpimpl::Endian::big );
    REQUIRE_STATIC( bitpacker::impl::get_type_array(BP_STRING("<u5"))[0].endian == bpimpl::Endian::little );
}

TEST_CASE("parse endianness correctly for many items", "[format]") {
    const auto types = bitpacker::impl::get_type_array(BP_STRING("<t3u1>s3b2"));
    REQUIRE_STATIC( types[0].endian == bpimpl::Endian::little );
    REQUIRE_STATIC( types[1].endian == bpimpl::Endian::little );
    REQUIRE_STATIC( types[2].endian == bpimpl::Endian::big );
    REQUIRE_STATIC( types[3].endian == bpimpl::Endian::big );
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
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[0].formatChar == 'u');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2<s12t10r10f32p2P2<"))[1].formatChar == 'f');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[2].formatChar == 'b');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2>"))[3].formatChar == 's');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("<u4f16b2s12t10r10f32p2P2"))[4].formatChar == 't');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10<f32p2P2"))[5].formatChar == 'r');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[6].formatChar == 'f');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16<b2s12t10r10f32p2P2"))[7].formatChar == 'p');
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4<f16b2>s12t10r10f32p2P2"))[8].formatChar == 'P');
}

TEST_CASE("parse bit count correctly", "[format]") {
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[0].count == 4);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2<s12t10r10f32p2P2<"))[1].count == 16);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[2].count == 2);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2>"))[3].count == 12);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("<u4f16b2s12t10r10f32p2P2"))[4].count == 10);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10<f32p2P2"))[5].count == 10);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[6].count == 32);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16<b2s12t10r10f32p2P2"))[7].count == 2);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4<f16b2>s12t10r10f32p2P2"))[8].count == 2);
}

TEST_CASE("parse bit offset correctly", "[format]") {
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[0].offset == 0);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2<s12t10r10f32p2P2<"))[1].offset == 4);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[2].offset == 4 + 16);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2>"))[3].offset == 4 + 16 + 2);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("<u4f16b2s12t10r10f32p2P2"))[4].offset == 4 + 16 + 2 + 12);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10<f32p2P2"))[5].offset == 4 + 16 + 2 + 12 + 10);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16b2s12t10r10f32p2P2"))[6].offset == 4 + 16 + 2 + 12 + 10 + 10);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4f16<b2s12t10r10f32p2P2"))[7].offset == 4 + 16 + 2 + 12 + 10 + 10 + 32);
    REQUIRE_STATIC(bitpacker::impl::get_type_array(BP_STRING("u4<f16b2>s12t10r10f32p2P2"))[8].offset == 4 + 16 + 2 + 12 + 10 + 10 + 32 + 2);
}

TEST_CASE("calculate format size in bits with bit-endian changing", "[format]") {
    constexpr unsigned format_size = 4 + 16 + 2 + 12 + 10 + 10 + 32 + 2 + 2;
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2<s12t10r10f32p2P2<")) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2>") ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("<u4f16b2s12t10r10f32p2P2") ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10<f32p2P2") ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16b2s12t10r10f32p2P2")  ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4f16<b2s12t10r10f32p2P2") ) == format_size );
    REQUIRE_STATIC(bitpacker::calcsize(BP_STRING("u4<f16b2>s12t10r10f32p2P2")) == format_size );
}

TEST_CASE("calculate format size in bytes with bit-endian changing", "[format]")
{
    constexpr unsigned format_size = 4 + 16 + 2 + 12 + 10 + 10 + 32 + 2 + 2;
    constexpr unsigned byte_size = (format_size / 8) + (format_size % 8 ? 1 : 0);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2s12t10r10f32p2P2"))   == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2<s12t10r10f32p2P2<")) == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2s12t10r10f32p2P2"))   == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2s12t10r10f32p2P2>"))  == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("<u4f16b2s12t10r10f32p2P2"))  == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2s12t10r10<f32p2P2"))  == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16b2s12t10r10f32p2P2"))   == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4f16<b2s12t10r10f32p2P2"))  == byte_size);
    REQUIRE_STATIC(bitpacker::calcbytes(BP_STRING("u4<f16b2>s12t10r10f32p2P2")) == byte_size);
}
