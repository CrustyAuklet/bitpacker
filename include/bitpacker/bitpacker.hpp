/**
 *  BITPACKER
 *  type-safe and low boilerplate bit-level serialization
 *  https://github.com/CrustyAuklet/bitpacker
 *
 *  Copyright 2020 Ethan Slattery
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>

#ifndef   bitpacker_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define bitpacker_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define bitpacker_CPLUSPLUS  __cplusplus
# endif
#endif

#define bitpacker_CPP98_OR_GREATER  ( bitpacker_CPLUSPLUS >= 199711L )
#define bitpacker_CPP11_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201103L )
#define bitpacker_CPP14_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201402L )
#define bitpacker_CPP17_OR_GREATER  ( bitpacker_CPLUSPLUS >= 201703L )
#define bitpacker_CPP20_OR_GREATER  ( bitpacker_CPLUSPLUS > 201703L )

#if bitpacker_CPP20_OR_GREATER && defined(__has_include )
# if __has_include( <span> )
#  define bitpacker_HAVE_STD_SPAN  1
# else
#  define bitpacker_HAVE_STD_SPAN  0
# endif
#else
# define  bitpacker_HAVE_STD_SPAN  0
#endif

#if bitpacker_HAVE_STD_SPAN
#include <span>
#else
#include "nonstd/span.hpp"
#endif

namespace bitpacker {
#if bitpacker_HAVE_STD_SPAN
    using std::span;
    using std::as_bytes;
#else
    using nonstd::span;
# if bitpacker_CPP17_OR_GREATER && !defined(BITPACKER_NO_STD_BYTE)
    using nonstd::as_bytes;
# endif
#endif

# if bitpacker_CPP17_OR_GREATER && !defined(BITPACKER_NO_STD_BYTE)
    using byte_type = std::byte;
#else
    using byte_type = uint8_t;
# endif

    using size_type = std::size_t;
    constexpr size_type ByteSize = sizeof(byte_type) * CHAR_BIT;
    static_assert(CHAR_BIT == 8, "The target system has bytes that are not 8 bits!");
    static_assert(static_cast<unsigned>(-1) == ~0U, "The target system is not 2's compliment! Default pack specializations will not work!");
    static_assert(
# if bitpacker_CPP17_OR_GREATER
            std::is_same<byte_type, std::byte>::value ||
#endif
            std::is_unsigned<byte_type>::value && std::is_integral<byte_type>::value && sizeof(byte_type) == 1U,
            "ByteType needs to be either std::byte or uint8_t");

    namespace impl {
        /// Represents a given bit offset
        struct Offset {
            size_type byte;
            size_type bit;
        };

        constexpr bool operator==(const Offset& lhs, const Offset& rhs) {
            return lhs.byte == rhs.byte && lhs.bit == rhs.bit;
        }
        constexpr bool operator>(const Offset& lhs, const Offset& rhs) {
            return lhs.byte > rhs.byte || ( lhs.byte == rhs.byte && lhs.bit > rhs.bit );
        }
        constexpr bool operator<(const Offset& lhs, const Offset& rhs) {
            return lhs.byte < rhs.byte || ( lhs.byte == rhs.byte && lhs.bit < rhs.bit );
        }
        constexpr bool operator!=(const Offset& lhs, const Offset& rhs) {
            return !(lhs == rhs);
        }
        constexpr bool operator>=(const Offset& lhs, const Offset& rhs) {
            return lhs > rhs || lhs == rhs;
        }
        constexpr bool operator<=(const Offset& lhs, const Offset& rhs) {
            return lhs < rhs || lhs == rhs;
        }

        /// get the byte and bit offset of a given bit number
        constexpr Offset get_offset(size_type offset) noexcept {
            return { (offset / ByteSize), (offset % ByteSize) };
        }
        /// create mask from the nth bit from the MSB to bit 0 (inclusive)
        constexpr byte_type right_mask(const size_type n) noexcept {
            return static_cast<byte_type>((1U << (8 - n)) - 1);
        }
        /// create mask from the nth bit from the MSB to bit 8 (inclusive)
        constexpr byte_type left_mask(const size_type n) noexcept {
            return static_cast<byte_type>(~static_cast<byte_type>(right_mask(n) >> 1U));
        }
    }   // implementation namespace

    template<typename ValueType>
    constexpr void pack_into(span<byte_type> buffer, size_type offset, size_type size, ValueType v) noexcept {
        static_assert( std::is_unsigned<ValueType>::value && std::is_integral<ValueType>::value, "ValueType needs to be an unsigned integral type");
        const auto start = impl::get_offset(offset);
        const auto end   = impl::get_offset(offset + size - 1);
        const byte_type startMask   = impl::right_mask(start.bit);    // mask of the start byte, 1s where data is
        const byte_type endMask     = impl::left_mask(end.bit);       // mask of the end byte, 1s where data is

        // mask off any bits outside the size of the actual field, if size < bits in ValueType
        // NOTE: it is UB to left shift ANY value if the shift is >= the bits in the value!
        // this also takes care of zero size values.
        if( size < sizeof(ValueType)*ByteSize ) {
            v &= static_cast<ValueType>(( ValueType{0x1U} << size) - 1);
        }

        if (start.byte == end.byte) {
            // case where start and end are in the same byte
            buffer[start.byte] &= static_cast<byte_type>(~( static_cast<uint8_t>(startMask & endMask)));
            buffer[start.byte] |= static_cast<byte_type>(v << (ByteSize - (end.bit + 1)));
        }
        else {
            // case where start and end are in different bytes
            buffer[end.byte] &= static_cast<byte_type>(~endMask);
            // TODO: simpler way to get shift. byte aligned data is a special case (%chunksize and the ternary)
            buffer[end.byte] |= static_cast<byte_type>(v << (ByteSize - ((end.bit+1)%ByteSize) ) % ByteSize);
            v >>= ((end.bit+1)%ByteSize) != 0 ? (end.bit+1)%ByteSize : ByteSize;

            for (size_type i = end.byte - 1; i > start.byte; --i) {
                buffer[i] = static_cast<byte_type>(v);
                // NOLINTNEXTLINE - this loop will NOT run 1-byte types
                v >>= ByteSize;
            }

            buffer[start.byte] &= static_cast<byte_type>(~startMask);
            buffer[start.byte] |= static_cast<byte_type>(v);
        }
    }

    template<typename ReturnType>
    constexpr ReturnType unpack_from(span<const byte_type> buffer, size_type offset, size_type size) noexcept {
        static_assert( std::is_unsigned<ReturnType>::value && std::is_integral<ReturnType>::value, "ReturnType needs to be an unsigned integral type");
        const auto start = impl::get_offset(offset);
        const auto end   = impl::get_offset(offset + size - 1);

        // case where size is zero
        if (size == 0) {
            return 0;
        }

        // case where the the entire field is in one byte
        if (start.byte == end.byte) {
            const size_type shift = (ByteSize - (end.bit + 1));
            // NOLINTNEXTLINE - size will always be <= 8 if we are within a byte!
            const size_type mask  = (1u << size) - 1;
            return static_cast<ReturnType>( static_cast<uint8_t>((buffer[start.byte]) >> shift) & mask );
        }

        // case where the field covers 2 or more bytes
        ReturnType value = static_cast<uint8_t>(buffer[start.byte]) & static_cast<uint8_t>(impl::right_mask(start.bit));
        for (size_type i = start.byte + 1; i < end.byte; ++i) {
            value = static_cast<ReturnType>(static_cast<ReturnType>(value << ByteSize) | static_cast<uint8_t>(buffer[i]));
        }
        const ReturnType shifted_end   = static_cast<ReturnType>(static_cast<uint8_t>(buffer[end.byte]) >> (ByteSize - (end.bit+1)));
        const ReturnType shifted_value = static_cast<ReturnType>(value << (end.bit + 1));
        return shifted_value | shifted_end;
    }

    /************************  Template specialization for unpacking  ***************************/

    template <typename T>
    constexpr T get(span<const byte_type> buffer, size_type offset) noexcept;

    template <>
    constexpr uint8_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return unpack_from<uint8_t>(buffer, offset, 8);
    }

    template <>
    constexpr int8_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return static_cast<int8_t>(unpack_from<uint8_t>(buffer, offset, 8));
    }

    template <>
    constexpr uint16_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return unpack_from<uint16_t>(buffer, offset, 16);
    }

    template <>
    constexpr int16_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return static_cast<int16_t>(unpack_from<uint16_t>(buffer, offset, 16));
    }

    template <>
    constexpr uint32_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return unpack_from<uint32_t>(buffer, offset, 32);
    }

    template <>
    constexpr int32_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return static_cast<int32_t>(unpack_from<uint32_t>(buffer, offset, 32));
    }

    template <>
    constexpr uint64_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return unpack_from<uint64_t>(buffer, offset, 64);
    }

    template <>
    constexpr int64_t get(span<const byte_type> buffer, size_type offset) noexcept {
        return static_cast<int64_t>(unpack_from<uint64_t>(buffer, offset, 64));
    }

    /*************************  Template specialization for packing  ****************************/

    template <typename T>
    constexpr void store(span<byte_type> buffer, size_type offset, T value) noexcept;

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, uint8_t value) noexcept {
        pack_into(buffer, offset, ByteSize, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, int8_t value) noexcept {
        pack_into(buffer, offset, ByteSize, static_cast<uint8_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, uint16_t value) noexcept {
        pack_into(buffer, offset, 16, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, int16_t value) noexcept {
        pack_into(buffer, offset, 16, static_cast<uint16_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, uint32_t value) noexcept {
        pack_into(buffer, offset, 32, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, int32_t value) noexcept {
        pack_into(buffer, offset, 32, static_cast<uint32_t>(value));
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, uint64_t value) noexcept {
        pack_into(buffer, offset, 64, value);
    }

    template<>
    constexpr void store(span<byte_type> buffer, size_type offset, int64_t value) noexcept {
        pack_into(buffer, offset, 64, static_cast<uint64_t>(value));
    }

} // namespace bitpacker
