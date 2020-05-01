#include "test_common.hpp"
#include "bitstream.h"
#include <array>

template <std::size_t SIZE>
void pack_unsigned_integer(std::array<uint8_t, SIZE>& data, const int offset, const int size, const unsigned value) {
    struct bitstream_writer_t writer{};
    struct bitstream_writer_bounds_t bounds{};
    bitstream_writer_init(&writer, data.data());
    bitstream_writer_bounds_save(&bounds, &writer, offset, size);
    bitstream_writer_seek(&writer, offset);
    bitstream_writer_write_u64_bits(&writer, value, size);
    bitstream_writer_bounds_restore(&bounds);
}

TEST_CASE("compare to bitstruct: pack aligned", "[bitpacker::bitstruct]") {
    std::array<uint8_t, 3> buff{0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 3> buff2{0xFF, 0xFF, 0xFF};
    const unsigned offset = 4;
    const unsigned size  = 4;
    const unsigned value = 5;

    bitpacker::insert(buff, offset, size, value);
    pack_unsigned_integer(buff2, offset, size, value);

    REQUIRE( buff == buff2 );
}

TEST_CASE("compare to bitstruct: unaligned", "[bitpacker::bitstruct]") {
    std::array<uint8_t, 3> buff{0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 3> buff2{0xFF, 0xFF, 0xFF};
    const unsigned offset = 4;
    const unsigned size  = 7;
    const unsigned value = 0x55;

    bitpacker::insert(buff, offset, size, value);
    pack_unsigned_integer(buff2, offset, size, value);

    REQUIRE( buff == buff2 );
}

TEST_CASE("compare to bitstruct: across 3 bytes", "[bitpacker::bitstruct]") {
    std::array<uint8_t, 4> buff{0xFF, 0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 4> buff2{0xFF, 0xFF, 0xFF, 0xFF};
    const unsigned offset = 4;
    const unsigned size  = 4+8+2;
    const unsigned value = 0x55;

    bitpacker::insert(buff, offset, size, value);
    pack_unsigned_integer(buff2, offset, size, value);

    REQUIRE( buff == buff2 );
}