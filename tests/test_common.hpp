
#include "span.hpp"

namespace bitpacker {
    using nonstd::span;
}

#include "bitpacker/bitpacker.hpp"

#include <iostream>
#include <sstream>
#include <cstddef>

inline std::string char_to_bits(const uint8_t v) {
    std::stringstream ss;
    for(unsigned mask = 0x80; mask > 0; mask >>= 1) {
        ss << ( v & mask ? "1" : "0" );
    }
    return ss.str();
}

#include <catch2/catch.hpp>

namespace Catch {
    template<std::size_t N>
    struct StringMaker<std::array<uint8_t, N>> {
        static std::string convert( std::array<uint8_t, N> const& arr ) {
            std::stringstream ss;
            ss << "{ ";
            for(const auto& i : arr) {
                ss << char_to_bits(i) << " ";
            }
            ss << " }";
            return ss.str();
        }
    };

#if __cpp_lib_byte
    template<std::size_t N>
    struct StringMaker<std::array<std::byte, N>> {
        static std::string convert( std::array<std::byte, N> const& arr ) {
            std::stringstream ss;
            ss << "{ ";
            for(const auto& i : arr) {
                ss << char_to_bits(static_cast<uint8_t>(i) ) << " ";
            }
            ss << " }";
            return ss.str();
        }
    };
#endif

    template<std::size_t N>
    struct is_range<std::array<uint8_t, N>> {
        static const bool value = false;
    };
}
