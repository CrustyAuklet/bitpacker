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
#ifndef BITPACKER_BITPACK_HPP
#define BITPACKER_BITPACK_HPP

#include "platform.hpp"

#include <cstdint>
#include <cstddef>
#include <climits>
#include <limits>
#include <type_traits>
#include <tuple>

#if bitpacker_HAVE_STD_SPAN
#include <span>
namespace bitpacker {
    using std::span;
    using std::as_bytes;
}
#elif bitpacker_HAVE_SPAN_LITE
#include <nonstd/span.hpp>
namespace bitpacker {
    using gsl::span;
}
#elif bitpacker_HAVE_GSL_LITE_SPAN
#include <gsl/gsl-lite.hpp>
namespace bitpacker {
    using gsl::span;
}
#elif bitpacker_HAVE_GSL_SPAN
#include <gsl/gsl>
namespace bitpacker {
    using gsl::span;
}
#endif

namespace bitpacker {

# if bitpacker_CPP17_OR_GREATER && defined(BITPACKER_USE_STD_BYTE)
    using byte_type = std::byte;
#else
    using byte_type = uint8_t;
# endif

    using size_type = std::size_t;
    constexpr size_type ByteSize = sizeof(byte_type) * CHAR_BIT;
    static_assert(CHAR_BIT == 8, "The target system has bytes that are not 8 bits!");
    static_assert(static_cast<unsigned>(-1) == ~0U, "The target system is not 2's compliment! Default pack specializations will not work!");
    static_assert(
#if bitpacker_CPP17_OR_GREATER && defined(BITPACKER_USE_STD_BYTE)
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

        /// returns true if the bit offset is byte aligned
        constexpr bool is_aligned(const size_type bit_offset) noexcept {
            return (bit_offset % ByteSize) == 0;
        }

        /// returns true if the offset is byte aligned
        constexpr bool is_aligned(const Offset offset) noexcept {
            return offset.bit == 0;
        }

        /// get the byte and bit offset of a given bit number
        constexpr Offset get_offset(size_type offset) noexcept {
            return { (offset / ByteSize), (offset % ByteSize) };
        }
        /// create mask from the nth bit from the MSB to bit 0 (inclusive)
        constexpr byte_type right_mask(const size_type n) noexcept {
            return static_cast< byte_type >((1U << (ByteSize - n)) - 1);
        }
        /// create mask from the nth bit from the MSB to bit 8 (inclusive)
        constexpr byte_type left_mask(const size_type n) noexcept {
            return static_cast<byte_type>(~static_cast<byte_type>(right_mask(n) >> 1U));
        }

        /// finds the smallest fixed width unsigned integer that can fit NumBits bits
        template <size_type NumBits>
        using unsigned_type = std::conditional_t<NumBits <= 8, uint8_t,
                std::conditional_t<NumBits <= 16, uint16_t,
                        std::conditional_t<NumBits <= 32, uint32_t,
                                std::conditional_t<NumBits <= 64, uint64_t,
                                        void >>>>;

        /// finds the smallest fixed width signed integer that can fit NumBits bits
        template <size_type NumBits>
        using signed_type = std::conditional_t<NumBits <= 8, int8_t,
                std::conditional_t<NumBits <= 16, int16_t,
                        std::conditional_t<NumBits <= 32, int32_t,
                                std::conditional_t<NumBits <= 64, int64_t,
                                        void >>>>;
#pragma warning(push)
#pragma warning(disable : 4293)
        /// sign extends an unsigned integral value to prepare for casting to a signed value.
        template<typename T, size_type BitSize>
        constexpr signed_type<BitSize> sign_extend(T val) noexcept
        {
            using return_type = signed_type<BitSize>;
            static_assert( std::is_unsigned<T>::value && std::is_integral<T>::value, "ValueType needs to be an unsigned integral type");
            // warning disabled for shifts bigger than type, since this if statement avoids that case.
            // if constexpr would work too, but trying to keep this section c++14 compatable
            if (BitSize < (sizeof(T)*ByteSize)) {
                const T upper_mask = static_cast<T>(~((static_cast<return_type>(1U) << BitSize) - 1));
                const T msb = static_cast<return_type>(1U) << (BitSize - 1);
                if (val & msb) { 
                    return static_cast<return_type>(val | upper_mask);
                }
            }
            return static_cast<return_type>(val);
        }
#pragma warning(pop)

        /// reverses the bits in the value `val`.
        template < typename T, size_type BitSize >
        constexpr auto reverse_bits(std::remove_cv_t< T > val) noexcept
        {
            using val_type = std::remove_reference_t< decltype(val) >;
            static_assert(std::is_integral<val_type>::value, "bitpacker::reverse_bits: val needs to be an integral type");
            using return_type = std::remove_reference_t< std::remove_cv_t< T >>;
            size_type count = BitSize-1;
            return_type retval = val & 0x01U;

            val >>= 1U;
            while (val && count) {
                retval <<= 1U;
                retval |= val & 0x01U;
                val >>= 1U;
                --count;
            }
            return retval << count; 
        }

    }  // implementation namespace

    /**
     * Inserts an unsigned integral value `v` into the byte buffer `buffer`. The value will overwrite
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer. Bits adjacent to this field will not be modified.
     * @tparam ValueType Type of the value `v` to insert. Must be an unsigned integral type.
     * @param buffer [IN/OUT] Span of bytes to insert the value `v` into
     * @param offset [IN] the bit offset to insert at. The value `v` will begin at this bit index
     * @param size [IN] the number of bits to use for inserting the value `v`. Must be <= 64.
     * @param v [IN] the value to insert into the byte container
     */
    template<typename ValueType>
    constexpr void insert(span<byte_type> buffer, size_type offset, size_type size, ValueType v) noexcept {
        static_assert( std::is_unsigned<ValueType>::value && std::is_integral<ValueType>::value, "bitpacker::insert : ValueType needs to be an unsigned integral type");
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
            // TODO: simpler way to get shift. byte aligned data is a special case (%ByteSize and the ternary)
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

    /**
     * Extracts an unsigned integral value from the byte buffer `buffer`. The value will be equal to
     * the bits from bit `offset` to `offset` + `size` counting from the most significant bit of the first byte
     * in the buffer.
     * @tparam ReturnType The return type of this function. Must be an unsigned integral type
     * @param buffer [IN] view of bytes to extract the value from. They will not be modified.
     * @param offset [IN] the bit offset to extract from. The return value will begin at this bit index.
     * @param size [IN] the number of bits to use, starting from `offset`, to construct the return value. Must be <= 64.
     * @return The unsigned integral value contained in `buffer` bit [`offset`, `offset`+`size`-1]. If ReturnType is
     *         not explicitly specified the smalled fixed width unsigned integer that can contain the value will be returned.
     */
    template<typename ReturnType>
    constexpr ReturnType extract(span<const byte_type> buffer, size_type offset, size_type size) noexcept {
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

    /*************************  Template specialization for packing  ****************************/

    template <typename T>
    constexpr void store(span<byte_type> buffer, size_type offset, T value) noexcept;

} // namespace bitpacker

#endif  // BITPACKER_BITPACK_HPP