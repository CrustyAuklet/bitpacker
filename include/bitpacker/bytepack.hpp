/**
 *  BYTEPACKER
 *  type-safe and low boilerplate bit-level serialization
 *  https://github.com/CrustyAuklet/bitpacker
 *
 *  Copyright 2020 Ethan Slattery
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BITPACKER_BYTEPACK_HPP
#define BITPACKER_BYTEPACK_HPP

#include "platform.hpp"

#include <cstdint>
#include <cstddef>
#include <climits>
#include <limits>
#include <type_traits>
#include <tuple>

#if bitpacker_CPP20_OR_GREATER && defined(__has_include )
#  if __has_include(<version>)
#    include <version>
#    if __cpp_lib_constexpr_algorithms >= 201806L
#      include <algorithm>
#      define BITPACKER_CONSTEXPR_ALGO 1
#    else
#      define BITPACKER_CONSTEXPR_ALGO 0
#    endif
#    if __cpp_lib_bitops >= 201907L
#      include <bit>
#      define BITPACKER_BIT_OPERATIONS 1
#    else
#      define BITPACKER_BIT_OPERATIONS 0
#    endif
#  else
// no version header
#  endif
#else
// not c++20 or no __has_include
#endif

#if bitpacker_HAVE_STD_SPAN
#include <span>
namespace bytepacker {
    using std::span;
    using std::as_bytes;
}
#elif bitpacker_HAVE_SPAN_LITE
#include <nonstd/span.hpp>
namespace bytepacker {
    using gsl::span;
}
#elif bitpacker_HAVE_GSL_LITE_SPAN
#include <gsl/gsl-lite.hpp>
namespace bytepacker {
    using gsl::span;
}
#elif bitpacker_HAVE_GSL_SPAN
#include <gsl/gsl>
namespace bytepacker {
    using gsl::span;
}
#endif

namespace bytepacker {

#if bytepacker_CPP17_OR_GREATER && defined(BITPACKER_USE_STD_BYTE)
    using byte_type = std::byte;
#else
    using byte_type = uint8_t;
#endif

#if BITPACKER_BIT_OPERATIONS
    constexpr bool little_endian_system = (std::endian::native == std::endian::little);
#else
    constexpr bool little_endian_system = true;
# endif

    using size_type = std::size_t;
    constexpr size_type ByteSize = sizeof(byte_type) * CHAR_BIT;
    static_assert(CHAR_BIT == 8, "The target system has bytes that are not 8 bits!");
    static_assert(static_cast<unsigned>(-1) == ~0U, "The target system is not 2's compliment! Default pack specializations will not work!");
    static_assert(
    #if bytepacker_CPP17_OR_GREATER && defined(bytepacker_USE_STD_BYTE)
        std::is_same<byte_type, std::byte>::value ||
    #endif
        std::is_unsigned<byte_type>::value && std::is_integral<byte_type>::value && sizeof(byte_type) == 1U,
        "ByteType needs to be either std::byte or uint8_t");

    namespace impl {
        // use C++20 if available, otherwise an intrinsic or pessimistic answer
        constexpr bool is_constant_evaluated() {
#if bitpacker_HAVE_IS_CONSTANT_EVALUTATED
            return std::is_constant_evaluated();
#elif defined(__GNUC__) // defined for both GCC and clang
            return __builtin_is_constant_evaluated();
#else // If the builtin is not available, return a pessimistic result and do constexpr way
            return true;
#endif
        }

#if defined(_MSC_VER) && !defined(_DEBUG)
        constexpr bool use_msvc_intrinsic = true;
#else
        constexpr bool use_msvc_intrinsic = false;
#endif
#if defined(__llvm__) || (defined(__GNUC__) && !defined(__ICC))
        constexpr bool use_gnu_intrinsic = true;
#else
        constexpr bool use_gnu_intrinsic = false;
#endif

        constexpr uint8_t SwapEndian(const std::uint8_t v) {
            return v;
        }

        constexpr uint16_t SwapEndian(const std::uint16_t value) {
            if(is_constant_evaluated() || !use_msvc_intrinsic) {
                const uint16_t Hi = value << 8U;
                const uint16_t Lo = value >> 8U;
                return Hi | Lo;
            }
#if defined(_MSC_VER) && !defined(_DEBUG)
            // The DLL version of the runtime lacks these functions (bug!?), but in a
            // release build they're replaced with BSWAP instructions anyway.
            return _byteswap_ushort(value);
#endif
        }

        constexpr uint32_t SwapEndian(const std::uint32_t value) {
            if(is_constant_evaluated() || (!use_msvc_intrinsic && !use_gnu_intrinsic)) {
                const uint32_t Byte0 = value & 0x000000FFU;
                const uint32_t Byte1 = value & 0x0000FF00U;
                const uint32_t Byte2 = value & 0x00FF0000U;
                const uint32_t Byte3 = value & 0xFF000000U;
                return (Byte0 << 24U) | (Byte1 << 8U) | (Byte2 >> 8U) | (Byte3 >> 24U);
            }
#if defined(__llvm__) || (defined(__GNUC__) && !defined(__ICC))
            return __builtin_bswap32(value);
#elif defined(_MSC_VER) && !defined(_DEBUG)
            return _byteswap_ulong(value);
#endif
        }

        constexpr uint64_t SwapEndian(const std::uint64_t value) {
            if(is_constant_evaluated() || (!use_msvc_intrinsic && !use_gnu_intrinsic)) {
                const uint64_t Hi = SwapEndian(static_cast<uint32_t>(value));
                const uint32_t Lo = SwapEndian(static_cast<uint32_t>(value >> 32U));
                return (Hi << 32U) | Lo;
            }
#if defined(__llvm__) || (defined(__GNUC__) && !defined(__ICC))
            return __builtin_bswap64(value);
#elif defined(_MSC_VER) && !defined(_DEBUG)
            return _byteswap_uint64(value);
#endif
        }

    }  // implementation namespace

    /**
     * Inserts an unsigned integral value `v` into the byte buffer `buffer`. The value will overwrite
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer. Bits adjacent to this field will not be modified.
     * @tparam ValueType Type of the value `v` to insert. Must be an integral type.
     * @tparam LittleEndian true if v should be inserted as little endian, otherwise insert as Big Endian.
     * @param buffer [IN/OUT] Span of bytes to insert the value `v` into
     * @param offset [IN] the byte offset to insert at
     * @param v [IN] the value to insert into the byte container
     */
    template<typename ValueType, bool LittleEndian = true>
    constexpr void insert(span<byte_type> buffer, size_type byte_offset, ValueType v) noexcept {
        static_assert(std::is_integral_v<ValueType>, "extract only works with integral values!");
        static_assert(LittleEndian, "BigEndian values not implemented");
        if(std::is_same_v<ValueType, uint8_t>) {
            buffer[byte_offset] = v;
        }
        else if constexpr (std::is_same_v<ValueType, uint16_t>) {
            if (LittleEndian) {
                buffer[byte_offset]   = v;
                buffer[byte_offset+1] = v >> 8U;
            } else {
                buffer[byte_offset]   = v >> 8U;
                buffer[byte_offset+1] = v;
            }
        }
        else if constexpr (std::is_same_v<ValueType, uint32_t>) {
            if (LittleEndian) {
                buffer[byte_offset]   = v;
                buffer[byte_offset+1] = v >> 8U;
                buffer[byte_offset+2] = v >> 16U;
                buffer[byte_offset+3] = v >> 24U;
            } else {
                buffer[byte_offset]   = v >> 24U;
                buffer[byte_offset+1] = v >> 16U;
                buffer[byte_offset+2] = v >> 8U;
                buffer[byte_offset+3] = v;
            }
        }
        else if constexpr (std::is_same_v<ValueType, uint64_t>) {
            if (LittleEndian) {
                buffer[byte_offset]   = v;
                buffer[byte_offset+1] = v >> 8U;
                buffer[byte_offset+2] = v >> 16U;
                buffer[byte_offset+3] = v >> 24U;
                buffer[byte_offset+4] = v >> 32U;
                buffer[byte_offset+5] = v >> 40U;
                buffer[byte_offset+6] = v >> 48U;
                buffer[byte_offset+7] = v >> 56U;
            } else {
                buffer[byte_offset]   = v >> 56U;
                buffer[byte_offset+1] = v >> 48U;
                buffer[byte_offset+2] = v >> 40U;
                buffer[byte_offset+3] = v >> 32U;
                buffer[byte_offset+4] = v >> 24U;
                buffer[byte_offset+5] = v >> 16U;
                buffer[byte_offset+6] = v >> 8U;
                buffer[byte_offset+7] = v;
            }
        }
    }

    /**
     * Extracts an unsigned integral value from the byte buffer `buffer`. The value will be equal to
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer.
     * @tparam ReturnType The return type of this function. Must be an integral type
     * @tparam LittleEndian true if the value should be extracted as little endian, otherwise extract as Big Endian.
     * @param buffer [IN] view of bytes to extract the value from. They will not be modified.
     * @param offset [IN] the bit offset to extract from. The return value will begin at this bit index.
     * @param size [IN] the number of bits to use, starting from `offset`, to construct the return value. Must be <= 64.
     * @return The unsigned integral value contained in `buffer` bit [`offset`, `offset`+`size`-1]. If ReturnType is
     *         not explicitly specified the smalled fixed width unsigned integer that can contain the value will be returned.
     */
    template<typename ReturnType, bool LittleEndian = true>
    constexpr ReturnType extract(span<const byte_type> buffer, size_type offset = 0) {
        static_assert(std::is_integral_v<ReturnType>, "extract only works with integral values!");
        if(std::is_same_v<std::make_unsigned_t<ReturnType>, uint8_t>) {
            return static_cast<ReturnType>(buffer[offset]);
        }
        if(std::is_same_v<std::make_unsigned_t<ReturnType>, uint16_t>) {
            uint16_t v{};
            if (LittleEndian) {
                v += static_cast<uint16_t>(static_cast<uint8_t>(buffer[offset+0]));
                v += static_cast<uint16_t>(static_cast<uint8_t>(buffer[offset+1]) << 8U);
            } else {
                v += static_cast<uint16_t>(static_cast<uint8_t>(buffer[offset+1]));
                v += static_cast<uint16_t>(static_cast<uint8_t>(buffer[offset+0]) << 8U);
            }
            return static_cast<ReturnType>(v);
        }
        if(std::is_same_v<std::make_unsigned_t<ReturnType>, uint32_t>) {
            uint32_t v{};
            if (LittleEndian) {
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+0]));
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+1]) << 8U);
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+2]) << 16U);
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+3]) << 24U);
            } else {
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+3]));
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+2]) << 8U);
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+1]) << 16U);
                v += static_cast<uint32_t>(static_cast<uint8_t>(buffer[offset+0]) << 24U);
            }
            return static_cast<ReturnType>(v);
        }
        if(std::is_same_v<std::make_unsigned_t<ReturnType>, uint64_t>) {
            uint64_t v{};
            if (LittleEndian) {
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+0]));
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+1]) << 8ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+2]) << 16ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+3]) << 24ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+4]) << 32ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+5]) << 40ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+6]) << 48ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+7]) << 56ULL);
            } else {
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+7]));
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+6]) << 8ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+5]) << 16ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+4]) << 24ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+3]) << 32ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+2]) << 40ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+1]) << 48ULL);
                v += static_cast<uint64_t>(static_cast<uint64_t>(buffer[offset+0]) << 56ULL);
            }
            return static_cast<ReturnType>(v);
        }
    }

} // namespace bytepacker

#endif // BITPACKER_BYTEPACK_HPP
