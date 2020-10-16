
#include <catch2/catch.hpp>

#include "span.hpp"
namespace bytepacker {
    using nonstd::span;
}
#include "bitpacker/bytepack.hpp"
#include <array>

template <std::size_t N>
bool compare_arrays( std::array<uint8_t, N> left, std::array<uint8_t, N> right ) {
    for(std::size_t idx = 0; idx < N; ++idx ) {
        if( left[idx] != right[idx]) {
            return false;
        }
    }
    return true;
}

TEST_CASE("pack within one byte", "[pack][bytepacker]") {
    std::array<uint8_t, 1> storage{};
    constexpr std::array<uint8_t, 1> expected{ 0x86 };
    uint8_t uval = 0;
    int8_t sval = 0;

    bytepacker::insert<uint8_t>(storage, 0, 134);
    std::memcpy(&uval, storage.data(), sizeof(uval));
    std::memcpy(&sval, storage.data(), sizeof(uval));
    REQUIRE(compare_arrays(storage, expected));
    REQUIRE( uval == 134U );
    REQUIRE( sval == -122 );

    // test round trip
    REQUIRE( bytepacker::extract<uint8_t>(storage, 0) == uval);
    REQUIRE( bytepacker::extract<int8_t>(storage, 0) == sval);
}

TEST_CASE("pack within two-byte value", "[pack][bytepacker]") {
    std::array<uint8_t, 2> storage{};
    constexpr std::array<uint8_t, 2> expected{ 0x9C, 0xCC };
    uint16_t uval = 0;
    int16_t sval = 0;

    bytepacker::insert<uint16_t>(storage, 0, 52380U);
    std::memcpy(&uval, storage.data(), sizeof(uval));
    std::memcpy(&sval, storage.data(), sizeof(uval));
    REQUIRE(compare_arrays(storage, expected));
    REQUIRE( uval == 52380U );
    REQUIRE( sval == -13156 );

    // test round trip
    REQUIRE( bytepacker::extract<uint16_t>(storage, 0) == uval);
    REQUIRE( bytepacker::extract<int16_t>(storage, 0) == sval);
}

TEST_CASE("pack within four-byte value", "[pack][bytepacker]") {
    std::array<uint8_t, 4> storage{};
    constexpr std::array<uint8_t, 4> expected{0xE4, 0x52, 0x97, 0x92 };
    uint32_t uval = 0;
    int32_t sval = 0;

    bytepacker::insert<uint32_t>(storage, 0, 2459390692UL);
    std::memcpy(&uval, storage.data(), sizeof(uval));
    std::memcpy(&sval, storage.data(), sizeof(uval));
    REQUIRE(compare_arrays(storage, expected));
    REQUIRE( uval == 2459390692UL );
    REQUIRE( sval == -1835576604L );

    // test round trip
    REQUIRE( bytepacker::extract<uint32_t>(storage, 0) == uval);
    REQUIRE( bytepacker::extract<int32_t>(storage, 0) == sval);
}

TEST_CASE("pack within four-byte value offset from start of buffer", "[pack][bytepacker]") {
    std::array<uint8_t, 6> storage{};
    constexpr std::array<uint8_t, 6> expected{ 0x00, 0x00, 0xE4, 0x52, 0x97, 0x92 };
    uint32_t uval = 0;
    int32_t sval = 0;

    bytepacker::insert<uint32_t>(storage, 2, 2459390692UL);
    std::memcpy(&uval, storage.data()+2, sizeof(uval));
    std::memcpy(&sval, storage.data()+2, sizeof(uval));
    REQUIRE(compare_arrays(storage, expected));
    REQUIRE( uval == 2459390692UL );
    REQUIRE( sval == -1835576604L );

    // test round trip
    REQUIRE( bytepacker::extract<uint32_t>(storage, 2) == uval);
    REQUIRE( bytepacker::extract<int32_t>(storage, 2) == sval);
}

TEST_CASE("pack within eight-byte value", "[pack][bytepacker]") {
    std::array<uint8_t, 8> storage{};
    constexpr std::array<uint8_t, 8> expected{ 0x92, 0x97, 0x52, 0xE4, 0x92, 0x97, 0x52, 0xE4 };
    uint64_t uval = 0;
    int64_t sval = 0;

    bytepacker::insert<uint64_t>(storage, 0, 16452379045889480594ULL);
    std::memcpy(&uval, storage.data(), sizeof(uval));
    std::memcpy(&sval, storage.data(), sizeof(uval));
    REQUIRE(compare_arrays(storage, expected));
    REQUIRE( uval == 16452379045889480594ULL );
    REQUIRE( sval == -1994365027820071022LL );

    // test round trip
    REQUIRE( bytepacker::extract<uint64_t>(storage, 0) == uval);
    REQUIRE( bytepacker::extract<int64_t>(storage, 0) == sval);
}
